/*
 * lcd.h
 */
 
#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>

void initLCD();
void delay(uint16_t x10ms);
void setArea(uint8_t xStart, uint8_t yStart, uint8_t xEnd, uint8_t yEnd);

uint8_t getScreenWidth();
uint8_t getScreenHeight();
void setOrientation(uint8_t orientation);

#define LONG_EDGE_PIXELS 160
#define SHORT_EDGE_PIXELS 160 // 128	I had a screen clearing issue so this is a temporary fix
#define LCD_OFFSET_HEIGHT 0
#define LCD_OFFSET_WIDTH 0

// orientation
#define ORIENTATION_VERTICAL 0
#define ORIENTATION_HORIZONTAL 1
#define ORIENTATION_VERTICAL_ROTATED 2
#define ORIENTATION_HORIZONTAL_ROTATED 3

// driver specific
#define SWRESET		0x01
#define	BSTRON		0x03
#define RDDIDIF		0x04
#define RDDST		0x09
#define SLEEPIN     0x10
#define	SLEEPOUT	0x11
#define	NORON		0x13
#define	INVOFF		0x20
#define INVON      	0x21
#define	SETCON		0x25
#define DISPOFF     0x28
#define DISPON      0x29
#define CASETP      0x2A
#define PASETP      0x2B
#define RAMWRP      0x2C
#define RGBSET	    0x2D
#define	MADCTL		0x36
#define SEP			0x37
#define	COLMOD		0x3A
#define DISCTR      0xB9
#define DOR			0xBA
#define	EC			0xC0
#define RDID1		0xDA
#define RDID2		0xDB
#define RDID3		0xDC
#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define SETOSC		0xB0
#define SETPWCTR4	0xB4
#define SETPWCTR5	0xB5
#define SETEXTCMD	0xC1
#define SETGAMMAP	0xC2
#define SETGAMMAN	0xC3

#endif /* LCD_H_ */
