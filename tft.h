/*
 * tft.h
 *
 *  Created on: Mar 23, 2023
 *      Author: dylan
 */

#ifndef TFT_H_
#define TFT_H_

#include "spi.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Initialize the connected ST7735 TFT display.
 */
void tft_init();

/**
 * Send a command to the connected ST7735 TFT display.
 * First argument is the command byte, second argument is an interger
 *   specifying how many parameters will follow, and then the rest of
 *   the varargs are those parameters.
 */
void tft_command(uint8_t cmd, size_t argc, ...);

/*
 * Set chip select low and select the TFT.
 */
void tft_select();

/*
 * Set chip select high and deselect the TFT.
 */
void tft_unselect();

/*
 * True / HIGH for data, false / LOW for command
 */
void tft_dc(bool dc);


// Commands
#define TFT_NOP 0x00
#define TFT_SWRESET 0x01
#define TFT_RDDID 0x04
#define TFT_RDDST 0x09
#define TFT_RDDPM 0x0A
#define TFT_RDDMADCTL 0x0B
#define TFT_RDDCOLMOD 0x0C
#define TFT_RDDIM 0x0D
#define TFT_RDDSM 0x0E
#define TFT_SLPIN 0x10
#define TFT_SLPOUT 0x11
#define TFT_PTLON 0x12
#define TFT_NORON 0x13
#define TFT_INVOFF 0x20
#define TFT_INVON 0x21
#define TFT_GAMSET 0x26
#define TFT_DISPOFF 0x28
#define TFT_DISPON 0x29
#define TFT_CASET 0x2A
#define TFT_RASET 0x2B
#define TFT_RAMWR 0x2C
#define TFT_RGBSET 0x2D
#define TFT_RAMRD 0x2E
#define TFT_PTLAR 0x30
#define TFT_TEOFF 0x34
#define TFT_TEON 0x35
#define TFT_MADCTL 0x36
#define TFT_IDMOFF 0x38
#define TFT_IDMON 0x39
#define TFT_COLMOD 0x3A
#define TFT_RDID1 0xDA
#define TFT_RDID2 0xDB
#define TFT_RDID3 0xDC


// Panel-specific commands (?)
#define TFT_FRMCTR1 0xB1
#define TFT_FRMCTR2 0xB2
#define TFT_FRMCTR3 0xB3
#define TFT_INVCTR 0xB4
#define TFT_PWCTR1 0xC0
#define TFT_PWCTR2 0xC1
#define TFT_PWCTR3 0xC2
#define TFT_PWCTR4 0xC3
#define TFT_PWCTR5 0xC4
#define TFT_VMCTR1 0xC5
#define TFT_VMOFCTR 0xC7
#define TFT_WRID2 0xD1
#define TFT_WRID3 0xD2
#define TFT_NVCTR1 0xD9
#define TFT_NVCTR2 0xDE
#define TFT_NVCTR3 0xDF
#define TFT_GAMCTRP1 0xE0
#define TFT_GAMCTRN1 0xE1


#ifdef __cplusplus
}
#endif
#endif /* TFT_H_ */
