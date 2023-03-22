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
 * 3. Start writing out the current video frame to the display via SPI.
 * 4. DMA-copy the leftover bits from the end of our current blocks to the beginning of the alternate buffer.
 * 5. Switch buffers, and repeat.
 */

#include <msp430.h>
#include <sdcard.hpp>

#include <Timing.h>
#include "spi.h"
#include "defines.h"
#include "lcd.h"

#define FRAME_SIZE 4414
#define AUDIO_FRAME_SIZE 1470
#define VIDEO_FRAME_SIZE 2944

uint8_t  __attribute__((persistent)) framebuffer_a[FRAME_SIZE] = { 0 };
uint8_t __attribute__((persistent)) framebuffer_b[FRAME_SIZE] = { 10 };
uint8_t __attribute__((persistent)) block_buffer[512] = { 0 };

// SRAM globals
uint16_t frame_number = 0;
uint16_t current_block = 0;
uint16_t current_block_offset = 0;
volatile uint8_t nextFrame = 0;

// Functions
bool read_frame(uint8_t *frame_buffer);

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
    BIS(P2DIR, BIT5); // P2.5 (display) as CS output. Active low.
    BIS(P2OUT, BIT5); // Pull lines high - chips aren't selected right now.
    BIS(P1DIR, BIT3); // SD card CS output, active low
    BIS(P1OUT, BIT3);

    BIS(P2DIR, BIT6 | BIT7); // p2.6/p2.7 are LED outputs
    BIC(P2OUT, BIT6 | BIT7);
    BIS(P3DIR, BIT6 | BIT7); // p3.6/p3.7 are also LEDs

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
    DMA0DA = (__SFR_FARPTR)(uint32_t)&TA1CCR2;
    DMA0SZ = AUDIO_FRAME_SIZE;
    // DMDA0SA is unset - main loop will set it and enable the transfer
}

/**
 * main.c
 */
void main(void) {
	msp_init();

	// Setup SD card
	if (!sd_init()) {
        displayNum(sd_errorCode);
        for (;;);
	}
    
    // First attempt is just going for audio - we're discarding the video data.
    // We still need to read the full frame, but we're only going to play the audio.
    // We'll also need to read the next frame's audio data into the alternate buffer.
    
    uint8_t *current_buffer = framebuffer_a;
    uint8_t *alternate_buffer = framebuffer_b;
    
    for (frame_number = 0; ; frame_number++) {
        // Read frame
        uint16_t start = millis();
        BIS(P2OUT, BIT6);
        if (!read_frame(alternate_buffer)) {
            for (;;);
        }
        BIC(P2OUT, BIT6);
        uint16_t x = millis() - start;
        displayNum(x);
//        displayNum(current_buffer[VIDEO_FRAME_SIZE]);
//        displayNum(frame_number / 30);

        // Delay until our next frame flag is set
        while (nextFrame == 0);
        nextFrame = 0;

        // Swap buffers
        uint8_t *tmp = current_buffer;
        current_buffer = alternate_buffer;
        alternate_buffer = tmp;


        // Reconfigure DMA0 to point at our new frame's audio buffer.
        DMA0SA = (__SFR_FARPTR)(uint32_t) (current_buffer + VIDEO_FRAME_SIZE);
        BIS(DMA0CTL, DMAABORT);
        BIC(DMA0CTL, DMAABORT);
        DMA0CTL |= DMAEN + DMAIE;
        
        // TODO: send display data in current_buffer here
    }
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


#pragma vector=TIMER0_A0_VECTOR
interrupt void frameInterrupt() {
    nextFrame = 1;
}

#pragma vector=TIMER0_B1_VECTOR
interrupt void audioGo() {
    P3OUT ^= BIT6;
    TB0IV = 0;
}
