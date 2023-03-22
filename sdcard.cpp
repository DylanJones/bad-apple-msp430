/*
 * sdcard.c
 *
 *  A very, very buggy and slow way to read data off of an SDHC/SDXC card
 *  over SPI.  That's right, we don't support standard SD and our "error handling"
 *  is atrocious!  Also we frequently get stuck in infinite loops here.
 *  Created on: Mar 18, 2023
 *      Author: dylan
 */

#include <msp430.h>
#include "sdcard.hpp"
#include "spi.h"
#include "defines.h"
#include "Timing.h"
#include "lcd.h"

// Globals (these are emulating SdFat's class member variables)
// Error code from last error
uint16_t sd_errorCode = SD_CARD_ERROR_INIT_NOT_CALLED;
// Error data from last error
uint16_t sd_status = 0;
// SD card type
uint8_t sd_cardType = 0;

static inline void sd_select() {
    BIC(P1OUT, BIT3);
}

static inline void sd_unselect() {
    BIS(P1OUT, BIT3);
}

/**
 * Byte for command, 4 bytes for argument.
 * Returns the first byte of the response (R1 byte)
 * or 0xFF if it times out.
 */
uint8_t sd_command(uint8_t cmd, uint32_t arg) {
    int i;
    uint8_t tx_buf[6] = { 0xFF };

    // Unpack arg
    tx_buf[0] = 0x40 | cmd; // why the | 0x40?
    for (i = 4; i > 0; i--) {
        tx_buf[i] = arg & 0xFF;
        arg >>= 8;
    }
    // Fake CRC determination - only matters for CMD0 and SEND_IF_COND
    tx_buf[5] = cmd == CMD0 ? 0x95 : 0x87;

    // Perform the SPI transaction
    sd_select(); // Make sure CS is low!
    spi_send(tx_buf, 6);

    // "Discard first byte to avoid MISO pull-up problems" - ???
    spi_receive_byte();

    // Wait for the first response byte.
    // All responses have the uppermost bit unset, so we can
    // just poll for that.
    i = 0;
    do {
        sd_status = spi_receive_byte();
    } while (sd_status & 0x80 && ++i < 15);
    return sd_status;
}

uint8_t sd_acmd(uint8_t acmd, uint32_t arg) {
    sd_command(CMD55, 0);
    return sd_command(acmd, arg);
}


bool sd_init() {
    // Following https://electronics.stackexchange.com/a/602106 very closely.
    //
    // SD card init sequence:
    // 0. Initialize SPI clock
    // 1. wait 5ms
    // 2. Send at least 16 bytes to a dummy slave
    // 3. Send CMD0 (go to idle state) until valid response received (0x01)
    //    If response is 0xFF, transmit more data or resend CMD0
    //    If response is 0x07, wrong CRC7 code was sent
    // 4. Send CMD8 (interface condition)
    uint8_t buf[16] = { 0xFF };
    uint16_t startTime;
    uint32_t arg;
    int i;

    delay(5);

    sd_unselect();
    spi_send(buf, 16);

    // Select the SD card
    sd_select();

    // Send CMD0 and wait for valid response
    for (i = 0; i <= SD_CMD0_RETRY; i++) {
        if (sd_command(CMD0, 0) == R1_IDLE_STATE) {
            break;
        }
        if (i == SD_CMD0_RETRY) {
            sd_errorCode = SD_CARD_ERROR_CMD0;
            goto fail;
        }
        // Force any active transfer to end for an already initialized card.
        for (uint8_t j = 0; j < 0xF; j++) {
          spi_receive(buf, 0xFF, 16);
        }
    }

    // Check card version
    if (!(sd_command(CMD8, 0x1AA) & R1_ILLEGAL_COMMAND)) {
        // Supports CMD8!  Check that we got our 0xAA byte echoed back.
        sd_cardType = SD_CARD_TYPE_SD2;
        for (uint8_t i = 4; i > 0; i--) {
            sd_status = spi_receive_byte();
        }
        // Check that we got the correct response echoed back
        if (sd_status != 0xAA) {
            sd_errorCode = SD_CARD_ERROR_CMD8;
        }
    } else {
        // Doesn't support CMD8
        sd_cardType = SD_CARD_TYPE_SD1;
    }

    // Next is ACMD41 - send operating conditions.
    // If it's type SD2, send the HCS bit to indicate SDHC support.
    startTime = millis();
    arg = sd_cardType == SD_CARD_TYPE_SD2 ? (1L << 30) : 0;
    while (sd_acmd(ACMD41, arg) != R1_READY_STATE) {
        if ((millis() - startTime) > SD_INIT_TIMEOUT) {
            sd_errorCode = SD_CARD_ERROR_ACMD41;
            goto fail;
        }
    }

    // For cards of type SD2, we have to send CMD58 (READ_OCR)
    // to figure out if it's a SDHC or not.
    if (sd_cardType == SD_CARD_TYPE_SD2) {
        if (sd_command(CMD58, 0)) {
            sd_errorCode = SD_CARD_ERROR_CMD58;
            goto fail;
        }
        if (spi_receive_byte() & 0xC0) {
            // SDHC bits are set - this is a SDHC card
            sd_cardType = SD_CARD_TYPE_SDHC;
        }
        // Discard the rest of the response (it's just voltage bits)
//        spi_receive(buf, 0xFF, 3);
        spi_receive_byte();
        spi_receive_byte();
        spi_receive_byte();
    }

    // TODO: increase SPI clock speed here in an idiomatic way
    UCB0BRW = 0; // until then: direct register tweaking
    // init successful!

    sd_unselect();
    return true;
fail:
    // bad :(
    sd_unselect();
    return false;
}


/**
 * Wait for a start block token, then read `size` bytes into `buf`.
 */
bool sd_read_data(uint8_t *buf, size_t size) {
    uint16_t start = millis();
    // Wait for data start token
    while ((sd_status = spi_receive_byte()) == 0xFF) {
        if (millis() - start > SD_READ_TIMEOUT) {
            sd_errorCode = SD_CARD_ERROR_READ_TIMEOUT;
            goto fail;
        }
    }

    // Confirm it was in fact the start token
    if (sd_status != DATA_START_SECTOR) {
        sd_errorCode = SD_CARD_ERROR_READ_TOKEN;
        goto fail;
    }

    // Receive the full block
    spi_receive(buf, 0xFF, size);

    // Discard CRC
    spi_receive_byte();
    spi_receive_byte();

    return true;

fail:
    sd_unselect(); // TODO: ???
    return false;
}

bool sd_read_block(uint32_t sector, uint8_t *buf) {
    // Non-SDHCs are indexed by bytes, not sectors.
    if (sd_cardType != SD_CARD_TYPE_SDHC) {
        sector <<= 9;
    }

    // CMD17 is the single read block command.
    if (sd_command(CMD17, sector)) {
        sd_errorCode = SD_CARD_ERROR_CMD17;
        goto fail;
    }

    if (!sd_read_data(buf, 512)) {
        goto fail;
    }

    sd_unselect();
    return true;

fail:
    sd_unselect();
    return false;
}
