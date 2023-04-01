/*
 * lcd.c
 */
 
#include "vendor_tft.h"
#include "vendor_ports.h"

uint8_t _orientation = 0;

uint8_t getScreenWidth() {
	if (_orientation == 0 || _orientation == 2)
		return SHORT_EDGE_PIXELS;
	else
		return LONG_EDGE_PIXELS;
}

uint8_t getScreenHeight() {
	if (_orientation == 0 || _orientation == 2)
		return LONG_EDGE_PIXELS;
	else
		return SHORT_EDGE_PIXELS;
}

void setArea(uint8_t xStart, uint8_t yStart, uint8_t xEnd, uint8_t yEnd) {
	writeCommand(CASETP);

	writeData(0);
	writeData(xStart);

	writeData(0);
	writeData(xEnd);

	writeCommand(PASETP);

	writeData(0);
	writeData(yStart);

	writeData(0);
	writeData(yEnd);

	writeCommand(RAMWRP);
	// data to follow
}

////////////////////////////////////
// gamma, lut, and other inits
////////////////////////////////////

void gammaAdjustmentST7735() {
	const uint8_t gmctrp1[] = {0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d,
		0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10};
	const uint8_t gmctrn1[] = {0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D,
		0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10};
// gamma correction is needed for accurate color but is not necessary.
	uint8_t c = 0;
	writeCommand(ST7735_GMCTRP1);// gamma adjustment (+ polarity)
	while (c < 16) {
		writeData(gmctrp1[c]);
		c++;
	}
	c = 0;
	writeCommand(ST7735_GMCTRN1); // gamma adjustment (- polarity)
	while (c < 16) {
		writeData(gmctrn1[c]);
		c++;
	}
}

void initLCD() {

	writeCommand(SWRESET);
	delay(20);
	writeCommand(SLEEPOUT);
	delay(20); 				// driver is doing self check, but seems
	writeCommand(COLMOD);	// to be working fine without the delay
	writeData(0x05);		// 16-bit mode
	writeCommand(MADCTL);
	setOrientation(1);
	gammaAdjustmentST7735();
	writeCommand(DISPON);// why? DISPON should be set on power up, sleep out should be enough
}

void setOrientation(uint8_t orientation) {

	writeCommand(MADCTL);

	switch (orientation) {
	case 1:
		writeData(0x68);
		_orientation = 1;
		break;
	case 2:
		writeData(0x08);
		_orientation = 2;
		break;
	case 3:
		writeData(0xA8);
		_orientation = 3;
		break;
	default:
		writeData(0xC8);
		_orientation = 0;
	}
}

// delay in increments of 10ms
void delay(uint16_t x10ms) {
	while (x10ms > 0) {
		__delay_cycles(10000);
		x10ms--;
	}
}
