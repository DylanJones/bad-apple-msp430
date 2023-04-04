/*
 * spi.c
 *
 *  Created on: Mar 18, 2023
 *      Author: dylan
 */
#include "spi.h"
#include "defines.h"
#include <stdbool.h>
#include <stdint.h>
#include <msp430.h>


void spi_init() {
    BIS(UCB0CTLW0, UCSWRST); // hold UCB0 logic in reset state while we're configuring stuff

    BIS(P1SEL0, BIT4);
    BIC(P1SEL1, BIT4); // Configure P1.4 as SPI CLK
    BIS(P1SEL0, BIT6);
    BIC(P1SEL1, BIT6); // Configure P1.6 as SIMO
    BIS(P1SEL0, BIT7);
    BIC(P1SEL1, BIT7); // Configure P1.7 as SOMI

    // Configure UCB0 as SPI.
    // Don't set UCCPKH/UCCPKL - we're in SPI mode 0 for the SD card.
    // UCMSB: SD cards transmit MSB first
    // UCMST: we're an SPI master.
    // UCSYNC: SPI requires this
    // UCSSEL__SMCLK: clock source is SMCLK
    // UCSWRST: keep us held in reset until everything's ready
    UCB0CTLW0 = UCMSB + UCMST + UCSYNC + UCSSEL__SMCLK + UCSWRST;
    UCB0BRW = 160; // prescaler of 160, for 100 kHz w/ 16 MHz SMCLK

    BIC(UCB0CTLW0, UCSWRST); // enable SPI - writes to UCB0TXBUF will start a transfer
}

static void dma_rx_setup(uint8_t *buf, size_t size) {
    // Setup DMA1 to receive
    DMACTL0 |= DMA1TSEL__UCB0RXIFG0;
    DMA1CTL = DMADT_0 + DMADSTINCR_3 + DMASRCINCR_0 + DMASRCBYTE + DMADSTBYTE;
    __data20_write_long((unsigned long)&DMA1SA, (unsigned long)&UCB0RXBUF);
    DMA1SZ = size;
    __data20_write_long((unsigned long)&DMA1DA, (unsigned long)buf);
}

void dma_tx_setup(const uint8_t *buf, size_t size) {
    // Setup DMA2 to transmit
    DMACTL1 |= DMA2TSEL__UCB0TXIFG0;
    DMA2CTL = DMADT_0 + DMADSTINCR_0 + DMASRCINCR_3 + DMASRCBYTE + DMADSTBYTE;
    __data20_write_long((unsigned long)&DMA2SA, (unsigned long)buf);
    DMA2SZ = size;
    __data20_write_long((unsigned long)&DMA2DA, (unsigned long)&UCB0TXBUF);
}


void spi_transaction(const uint8_t *output, uint8_t *input, size_t size) {
    unsigned int i;
    // TODO: use DMA instead
    for (i = 0; i < size; i++) {
        input[i] = spi_send_byte(output[i]);
    }
}

void spi_send(const uint8_t *output, size_t size) {
    unsigned int i;
    // TODO: use DMA instead
    for (i = 0; i < size; i++) {
        spi_send_byte(output[i]);
    }
    return;
}

void spi_send_dma(const uint8_t *output, size_t size) {
    dma_tx_setup(output, size);
    // start 'em up
    dmaDone = 0;
    DMA2CTL |= DMAEN + DMAIE;
    UCB0IFG &= ~(UCTXIFG | UCRXIFG);
    UCB0IFG |= UCTXIFG | UCRXIFG;
    // wait for DMAs to finish
    while(!dmaDone);
    // disable DMAs
    BIC(DMA2CTL, DMAEN + DMAIE);
}

void spi_receive(uint8_t *input, uint8_t fillByte, size_t size) {
    size_t i;
    for (i = 0; i < size; i++) {
        input[i] = spi_send_byte(fillByte);
    }
    return;
}

void spi_receive_dma(uint8_t *input, uint8_t fillByte, size_t size) {
    // TODO: Clean this up and make it easier to use in other functions!
     dma_rx_setup(input, size);
     dma_tx_setup(&fillByte, size - 1);
     // switch it around so it just repeats fillByte instead
     DMA2CTL = DMADT_0 + DMADSTINCR_0 + DMASRCINCR_0 + DMASRCBYTE + DMADSTBYTE;
     // start 'em up
     dmaDone = 0;
     DMA1CTL |= DMAEN;
     DMA2CTL |= DMAEN + DMAIE;
     UCB0TXBUF = 0xFF;
     // wait for DMAs to finish
     while(!dmaDone);
     // disable DMAs
     BIC(DMA1CTL, DMAEN);
     BIC(DMA2CTL, DMAEN + DMAIE);
}

uint8_t spi_send_byte(uint8_t byte) {
    UCB0TXBUF = byte;
    while (UCB0STATW & UCBUSY); // wait for SPI transaction to finish
    return UCB0RXBUF;
}

uint8_t spi_receive_byte() {
    return spi_send_byte(0xFF);
}


volatile bool dmaDone = 0;

#pragma vector=DMA_VECTOR
__interrupt void dmaInterrupt() {
    dmaDone = 1;
    DMAIV = 0;
}
