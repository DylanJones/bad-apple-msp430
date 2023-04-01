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
/*
 * True / HIGH for data, false / LOW for command
 */
void tft_send_data(bool dc) {
    if (dc) {
        BIS(P2OUT, BIT2);
    } else {
        BIC(P2OUT, BIT2);
    }
}


void tft_init1() {
    // Based off Adafruit init sequence
    tft_command(TFT_SWRESET, 0); // Issue software reset
    delay(50);
    tft_command(TFT_SLPOUT, 0); // Exit sleep mode
    delay(120);
    tft_command(TFT_COLMOD, 1, 3); // Set pixel format to 12 bits per pixel
    tft_command(TFT_COLMOD, 1, 5); // Set pixel format to 12 bits per pixel
    // Frame rate control:
    // 0 = fastest refresh, 6 lines front porch, 3 lines back porch
    tft_command(TFT_FRMCTR1, 3, 0, 6, 3);
    tft_command(TFT_MADCTL, 1, 0x68); // copied one
    // DISSET5
    tft_command(0xB6, 2,  0x15, 0x02);
    delay(10);
    tft_command(TFT_INVCTR, 1, 0);
    tft_command(TFT_PWCTR1, 2, 0x02, 0x70);
    tft_command(TFT_PWCTR2, 1, 0x05);
    tft_command(TFT_PWCTR3, 2, 0x01, 0x02);
    tft_command(TFT_VMCTR1, 2, 0x3C, 0x38);
    // PWCTR6
    tft_command(0xFC, 2, 0x11, 0x15);
    delay(50);

    // Gamma adjust 1
    tft_command(TFT_GAMCTRP1, 16, 0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d,
        0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10);
    // Gamma adjust 2
    tft_command(TFT_GAMCTRN1, 16, 0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D,
        0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10);

    // Column addr set: XSTART = 2, XEND = 129 (b/c of the porch we configured earlier)
    tft_command(TFT_CASET, 4, 0, 2, 0, 129);
//    // Row addr set: XSTART = 1, XEND = 160 (again b/c of porch)
    tft_command(TFT_RASET, 4, 0, 1, 0, 160);

    tft_command(TFT_IDMOFF, 0);
    delay(120);
    tft_command(TFT_NORON, 0);
    delay(120);
    tft_command(TFT_DISPON, 0); // Turn on display
    delay(100);

    tft_unselect();
    tft_send_data(true);
}

void tft_init() {
    // Based off ATTiny init sequence
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
    tft_send_data(true);
}

void tft_init3() {
    // Based off Rust init sequence
    tft_command(TFT_SWRESET, 0); // Issue software reset
    delay(200);
    tft_command(TFT_SLPOUT, 0); // Exit sleep mode
    delay(200);
    tft_command(TFT_IDMOFF, 0);
    tft_command(TFT_NORON, 0);
    // Frame rate control:
    // 0 = fastest refresh, 6 lines front porch, 3 lines back porch
    tft_command(TFT_FRMCTR1, 3, 0x01, 0x2C, 0x2D);
    tft_command(TFT_FRMCTR2, 3, 0x01, 0x2C, 0x2D);
    tft_command(TFT_FRMCTR3, 6, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D);
    // DISSET5
//    tft_command(0xB6, 2,  0x15, 0x02);
    tft_command(TFT_INVCTR, 1, 0x07);
    tft_command(TFT_PWCTR1, 2, 0xA2, 0x02, 0x84);
    tft_command(TFT_PWCTR2, 1, 0xC5);
    tft_command(TFT_PWCTR3, 2, 0x0A, 0x00);
    tft_command(TFT_PWCTR4, 2, 0x8A, 0x2A);
    tft_command(TFT_PWCTR5, 2, 0x8A, 0xEE);
    tft_command(TFT_VMCTR1, 1, 0x0E);
    // PWCTR6
//    tft_command(0xFC, 2, 0x11, 0x15);
    delay(50);

    // Gamma adjust 1
    tft_command(TFT_GAMCTRP1, 16, 0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d,
        0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10);
    // Gamma adjust 2
    tft_command(TFT_GAMCTRN1, 16, 0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D,
        0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10);

    // Column addr set: XSTART = 2, XEND = 129 (b/c of the porch we configured earlier)
//    tft_command(TFT_CASET, 4, 0, 2, 0, 129);
//    // Row addr set: XSTART = 1, XEND = 160 (again b/c of porch)
//    tft_command(TFT_RASET, 4, 0, 1, 0, 160);

    tft_command(TFT_MADCTL, 1, 0x68); // copied one
    tft_command(TFT_COLMOD, 1, 0x05); // Set pixel format to 12 bits per pixel
    tft_command(TFT_DISPON, 0); // Turn on display
    delay(200);

    tft_unselect();
    tft_send_data(false);
}



void tft_command(uint8_t cmd, size_t argc, ...) {
    va_list argptr;
    va_start(argptr, argc);
    tft_select();
    tft_send_data(false);
    spi_send_byte(cmd);
    tft_send_data(true);
    for (; argc > 0; argc--) {
        spi_send_byte((uint8_t) va_arg(argptr, uint16_t));
    }
    va_end(argptr);
}
