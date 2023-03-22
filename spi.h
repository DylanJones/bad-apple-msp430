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
#include "defines.h"


void spi_init();
void spi_transaction(const uint8_t *output, uint8_t *input, size_t size);
void spi_send(const uint8_t *output, size_t size);
void spi_receive(uint8_t *input, uint8_t fillByte, size_t size);
uint8_t spi_send_byte(uint8_t byte);
uint8_t spi_receive_byte();

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* SPI_H_ */
