/**
 * badapple.c
 * Display bad apple music video + song on a connected SPI TFT display, reading
 * the encoded song and video off of an SD card.
 *
 * Program overview:
 * 1. Initialize clocks, SPI
 * 2. Setup audio player: Timer A1 runs as fast as possible to emulate a PWM DAC.
 *        TA1's period is fixed by TA1CCR0, duty cycle controlled by TA1CCR1.
 *    Timer B1 drives DMA0 to transfer data from AUDIO_BUF_A to TA1CCR1.
 *        DMA0's source will be changed / reset by main loop.
 * 3. Initialize the SD card.
 * 4. Initialize the SPI display.
 *
 *
 * Main loop:
 * 1. Read one frame and write it to FRAM buffer A. (May overrun a little bit - make sure to keep hold of it)
 * 2. Reconfigure DMA0 to point to our newly acquired audio buffer - this will start playing the current frame's worth of audio.
 * 3. Simultaneously decode and write out the newly acquired framebuffer to the display via SPI.
 * 4. Switch buffers, and repeat.
 */

#include <msp430.h>
#include <sdcard.h>
#include <Timing.h>
#include "asm_funcs.h"
#include "spi.h"
#include "defines.h"
#include "lcd.h"
#include "tft.h"


#define AUDIO_FRAME_SIZE 1470
#define VIDEO_FRAME_SIZE 2560
#define FRAME_SIZE (AUDIO_FRAME_SIZE + VIDEO_FRAME_SIZE)

uint8_t  __attribute__((persistent)) framebuffer_a[FRAME_SIZE] = { 0 };
uint8_t __attribute__((persistent)) framebuffer_b[FRAME_SIZE] = { 0 };
uint8_t __attribute__((persistent)) block_buffer[512] = { 0 };

// SRAM globals
uint16_t frame_number = 0;
uint32_t current_block = 0;
uint16_t current_block_offset = 0;
volatile bool nextFrame = 0;

// Functions
bool read_frame(uint8_t *frame_buffer);
void decode_and_write_frame(uint8_t *current_buffer);
void memcpy_dma(void *dst, void *src, size_t size);


/**
 * MSP init: initialize misc. peripherals and pins
 */
void msp_init() {
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;         // Unlock ports from power manager
    // make the ULP advisor shut up
    PADIR = PBDIR = PCDIR = PDDIR = PEDIR = 0;
    PAOUT = PBOUT = PCOUT = PDOUT = PEOUT = 0;

    aclk_init();
    smclk_init();
    delay_init();
    lcd_init();
    spi_init();

    // Pins:
    __disable_interrupt();
    BIC(P2DIR, BIT1 | BIT2); // button P2.1 and P2.2 for seeking
    BIS(P2IE, BIT1 | BIT2);
    BIS(P2IES, BIT1 | BIT2);
    BIS(P2REN, BIT1 | BIT2);
    BIC(P2IFG, BIT1 | BIT2);
    __enable_interrupt();

    BIS(P2DIR, BIT2 | BIT6 | BIT7);
    BIS(P2OUT, BIT2 | BIT6 | BIT7);
    BIS(P3DIR, BIT6 | BIT7);
    BIS(P3OUT, BIT6 | BIT7);

    // Timer A1: PWM DAC
    // Blisteringly fast 16MHz count, 6-bit PWM for a frequency of ~250kHz
    BIS(P4DIR, BIT7); // P4.7 is TA1.2 output
    BIS(P4SEL0, BIT7);
    BIS(P4SEL1, BIT7);
    TA1EX0 = TAIDEX_0; // divide by 1
    TA1CTL = TASSEL__SMCLK | MC__UP | TACLR + ID__1; // SMCLK, up mode, clear timer, divide by 1
    TA1CCR0 = 0x3F; // 6-bit PWM
    TA1CCR2 = 0x2F; // 0% duty cycle
    TA1CCTL2 = OUTMOD_3;
    
    // Timer A0: count when it's time for the next frame
    TA0EX0 = TAIDEX_1; // divide by 2
    TA0CTL = TASSEL__SMCLK | MC__UP | TACLR | ID__8; // 1us timer ticks
    TA0CCR0 = 33333; // 30 Hz / FPS
    TA0CCTL0 = CCIE;

    // Timer B1: DMA0 trigger
    // We need a frequency of 44100 Hz (~22.6 uS per sample)
    // At 16MHz, this is 363.6 cycles per sample (we'll round up to 364)
    TB0EX0 = TBIDEX_0; // divide by 1
    TB0CTL = TBSSEL__SMCLK | MC__UP | TBCLR | ID__1; // SMCLK, up mode, clear timer, divide by 1
    TB0CCR0 = 364; // 364 cycles per sample
    TB0CCR0 = 361; // 364 cycles per sample
    
    // DMA0: Audio buffer to TA0CCR1
    DMACTL0 |= DMA0TSEL__TB0CCR0;
    DMA0CTL = DMADT_0 + DMADSTINCR_0 + DMASRCINCR_3 + DMASRCBYTE + DMADSTBYTE + DMALEVEL;
//    DMA0DA = (__SFR_FARPTR)(uint32_t)&TA1CCR2;
    __data20_write_long((uint32_t)&DMA0DA, (uint32_t)&TA1CCR2);
    DMA0SZ = AUDIO_FRAME_SIZE;
    // DMDA0SA is unset - main loop will set it and enable the transfer
}

/**
 * Main loop!
 */
void main(void) {
	msp_init();

//	displayNum(asmfunc("test 4"));
//	for(;;);

	// Setup SD card
	if (!sd_init()) {
        displayNum(sd_errorCode);
        for (;;);
	}

	// Setup TFT
    tft_init();
    
    uint8_t *current_buffer = framebuffer_a;
    uint8_t *alternate_buffer = framebuffer_b;
    uint16_t start = 0;
    
    for (frame_number = 0; ; frame_number++) {
        // Read frame
        BIS(P2OUT, BIT6);
        if (!read_frame(alternate_buffer)) {
            for (;;);
        }
        BIC(P2OUT, BIT6);

        // Display frame time

        // Delay until our next frame flag is set
        uint16_t x = millis() - start;
        displayNum(x);
        while (!nextFrame) __low_power_mode_1();
        nextFrame = 0;
        start = millis();


        // Swap buffers
        uint8_t *tmp = current_buffer;
        current_buffer = alternate_buffer;
        alternate_buffer = tmp;

        // Reconfigure DMA0 to point at our new frame's audio buffer.
        BIS(DMA0CTL, DMAABORT);
        DMA0SA = (current_buffer + VIDEO_FRAME_SIZE);
        BIC(DMA0CTL, DMAABORT);
        DMA0CTL |= DMAEN;
        
        tft_command(TFT_CASET, 4, 0, 0, 0, 128);
        tft_command(TFT_RASET, 4, 0, 0, 0, 160);
        tft_command(TFT_MADCTL, 1, 0x40);
        decode_and_write_frame(current_buffer);
    }
}

/**
 * This pragma causes the function to be copied into SRAM and executed
 * from there.  Since we're executing at SMCLK = 16MHz, we need 1
 * wait-state for every FRAM access which theoretically slows down
 * code.  By copying the most critical functions into RAM, we're able
 * to go from ~21ms to 18ms execution time on this function.
 */
#pragma CODE_SECTION (decode_and_write_frame, ".TI.ramfunc")
void decode_and_write_frame(uint8_t *current_buffer) {
    unsigned int i, j, k;
    /// Decode and send frame
    static const uint16_t lookup[2] = {0x0000, 0xFFFF};
    uint8_t *f = current_buffer;
    // Signal the start of our write to frame memory
    tft_command(TFT_RAMWR, 0);
    static const unsigned int lsize = 160;
    static const unsigned int csize = 128;
    uint16_t line_a[csize];
    uint16_t line_b[csize];
    uint16_t *line = line_a;

    for (i = 0; i < lsize; i++) {
        line = (line == line_a) ? line_b : line_a;
        for (j = 0; j < csize / 8; j++) {
            uint8_t packed = *f++;
            for (k = 0; k < 8; k++) {
                line[j * 8 + 7 - k] = lookup[(packed & 1)];
                packed >>= 1;
            }
        }
        if (i != 0) {
            // wait for DMA to finish
            while (!dmaDone);
        } else {
            // Setup the DMAs to transfer lines in the background
            dma_tx_setup((uint8_t *)line, csize * 2);
        }
        dmaDone = 0;
        __data20_write_long((unsigned long)&DMA2SA, (unsigned long)line);
        DMA2CTL |= DMAEN + DMAIE;
        UCB0IFG &= ~(UCTXIFG | UCRXIFG);
        UCB0IFG |= UCTXIFG | UCRXIFG;
//        spi_send_dma(line, csize * 2);
    }
    // wait for DMA to finish
    while (!dmaDone);
    DMA2CTL = 0;
    tft_unselect();
}


/**
 *  Read in a single frame of audio + video data from the SD card.
 *  Assumes that the current block is already loaded into 
 *  block_buffer (this is covered by previous invocation of this function).
 *  
 *  @param frame_buffer The buffer to read the frame into.
 */
bool read_frame(uint8_t *frame_buffer) {
    uint16_t bytes_remaining = FRAME_SIZE;
    // We can assume that block_buffer is already populated with the current block, from the previous read.
    do {
        // Copy the remaining bytes from the current block into the frame buffer.
        uint16_t bytes_to_copy = MIN(bytes_remaining, 512 - current_block_offset);
        memcpy(frame_buffer, block_buffer + current_block_offset, bytes_to_copy);
        bytes_remaining -= bytes_to_copy;
        frame_buffer += bytes_to_copy;
        current_block_offset += bytes_to_copy;

        // If we've reached the end of the block, read the next one.
        if (current_block_offset == 512) {
            current_block_offset = 0;
            current_block++;
            if (!sd_read_block(current_block, block_buffer)) {
                return false;
            }
        }
    } while (bytes_remaining > 0);
    return true;
}

void memcpy_dma(void *dst, void *src, size_t size) {
    DMA1CTL = 0;
    __data20_write_long(&DMA1SA, src);
    __data20_write_long(&DMA1DA, dst);
    DMA1SZ = size / 2;
    DMACTL0 |= DMA1TSEL__DMAREQ;
    DMA1CTL = DMADT_1 + DMAEN + DMALEVEL + DMAREQ + DMAIE;
    // DMA in block mode is blocking, so it's done by now.
    while (!dmaDone);
    DMA1CTL = 0; // Disarm it
    if (size & 1) { // Deal w/ odd numbers of bytes
        ((uint8_t*)dst)[size - 1] = ((uint8_t*)src)[size - 1];
    }
}


#pragma vector=TIMER0_A0_VECTOR
interrupt void frameInterrupt() {
    nextFrame = true;
    TA0IV = 0;
    __low_power_mode_off_on_exit();
}

#pragma vector=PORT2_VECTOR
interrupt void buttonInterrupt() {
    P2IV = 0;
    return;
    switch (P2IV) {
    case P2IV_P2IFG1:
        // Beginning of video
        current_block = 0;
        current_block_offset = 0;
        break;
    case P2IV_P2IFG2:
        current_block = 51728;
        current_block_offset = 424;
        break;
    default:
        break;
    }
    P2IV = 0;
}
