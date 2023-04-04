/*
 * sdcard.h
 *
 *  Created on: Mar 18, 2023
 *      Author: dylan
 */

#ifndef SDCARD_H_
#define SDCARD_H_
#include "defines.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Most recent SD card error code (think errno but for the SD card)
extern uint16_t sd_errorCode;
// Most recent R1 byte - see SD spec for bit field meanings
extern uint16_t sd_status;
// Connected SD card type
extern uint8_t sd_cardType;

/**
 * Byte for command, 4 bytes for argument.
 * Returns the first byte of the response (R1 byte)
 * or 0xFF if it times out.
 */
uint8_t sd_command(uint8_t cmd, uint32_t arg);

/**
 * Execute application-specific command acmd with
 * 4-byte argument arg, then return the R1 byte of the result.
 */
uint8_t sd_acmd(uint8_t acmd, uint32_t arg);

/**
 * Read a single 512 byte block into `buf`, at the index
 * `sector`.
 */
bool sd_read_block(uint32_t sector, uint8_t *buf);

/**
 * Initialize the SD card, returning true if the initialization failed.
 */
bool sd_init();


#ifdef __cplusplus
}
#endif
#endif /* SDCARD_H_ */
