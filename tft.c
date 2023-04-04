/*
 * tft.c
 *
 *  Created on: Mar 23, 2023
 *      Author: dylan
 */

#include <tft.h>
#include <stdarg.h>
#include <msp430.h>
#include <stdbool.h>
#include "defines.h"
#include "Timing.h"

void tft_select() {
    BIC(P2OUT, BIT6);
}

void tft_unselect() {
    BIS(P2OUT, BIT6);
}

void tft_dc(bool dc) {
    if (dc) {
        BIS(P2OUT, BIT2);
    } else {
        BIC(P2OUT, BIT2);
    }
}

void tft_init() {
    // Based off ATTiny init sequence
    // (other, longer and more proper init sequences do exist - check
    //  Adafruit's ST7735 library or the git history)
    tft_command(TFT_SWRESET, 0); // Issue software reset
    delay(150);
    tft_command(TFT_SLPOUT, 0); // Exit sleep mode
    delay(500);
    tft_command(TFT_COLMOD, 1, 0x05); // Set pixel format to 16 bits per pixel
    delay(50);
    tft_command(TFT_MADCTL, 1, 0x68); // copied one
    delay(10);
    tft_command(TFT_DISPON, 0); // Turn on display
    delay(100);
    tft_unselect();
    tft_dc(true);
}

void tft_command(uint8_t cmd, size_t argc, ...) {
    va_list argptr;
    va_start(argptr, argc);
    tft_select();
    tft_dc(false);
    spi_send_byte(cmd);
    tft_dc(true);
    for (; argc > 0; argc--) {
        spi_send_byte((uint8_t) va_arg(argptr, uint16_t));
    }
    va_end(argptr);
}
