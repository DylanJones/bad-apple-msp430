/*
 * sdcard.h
 *
 *  Created on: Mar 18, 2023
 *      Author: dylan
 */

#ifndef SDCARD_HPP_
#define SDCARD_HPP_

#include <stdint.h>
#include "SdInfo.hpp"

extern uint16_t sd_errorCode;
extern uint16_t sd_status;
extern uint8_t sd_cardType;

uint8_t sd_command(uint8_t cmd, uint32_t arg);
uint8_t sd_acmd(uint8_t acmd, uint32_t arg);
bool sd_read_block(uint32_t sector, uint8_t *buf);
bool sd_init();

#endif /* SDCARD_HPP_ */
