/*
 * spi.h
 *
 *  Created on: Mar 18, 2023
 *      Author: dylan
 */

#ifndef SPI_H_
#define SPI_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "defines.h"

/**
 * Initialize the UCB0 SPI peripheral.
 */
void spi_init();

/**
 * One SPI transaction: shift out the bytes on *output*, while reading the results to the buffer *input*.
 * Both buffers are the same size, specified by parameter `size`.
 */
void spi_transaction(const uint8_t *output, uint8_t *input, size_t size);

/**
 * Send size bytes from output, ignoring any incoming received bytes.
 */
void spi_send(const uint8_t *output, size_t size);

/*
 * Blocking DMA version of spi_send.
 * TODO: merge spi_send and spi_send_dma
 */
void spi_send_dma(const uint8_t *output, size_t size);

/*
 * Receive size bytes into input, repeating the same fillByte on the output.
 */
void spi_receive(uint8_t *input, uint8_t fillByte, size_t size);

/**
 * Blocking DMA version of spi_receive.
 * TODO: merge spi_receive and spi_receive_dma
 */
void spi_receive_dma(uint8_t *input, uint8_t fillByte, size_t size);

/**
 * Send the single byte `byte` and return the received byte.
 */
uint8_t spi_send_byte(uint8_t byte);

/**
 * Wrapper around spi_send_byte that sends 0xFF and returns the received byte.
 */
uint8_t spi_receive_byte();

/**
 * Prepare DMA2 to perform a block -> single address bytewise
 * transfer, but don't start the DMA yet.
 */
void dma_tx_setup(const uint8_t *buf, size_t size);

// Unfortunate hack: this flag is to true / 1 whenever a DMA completes
// and triggers the DMA_VECTOR ISR.
extern volatile bool dmaDone;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* SPI_H_ */
