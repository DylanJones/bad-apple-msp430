/*
 * Timer.hpp
 *
 *  Created on: Mar 1, 2023
 *      Author: dylan
 *
 *  Delay and millis()!
 */

#ifndef TIMING_H_
#define TIMING_H_

#include <msp430.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*****
 * API
 *****/

/**
 * Typedef to make it more explicit when we're dealing with milliseconds
 */
//typedef uint16_t millis_t;
typedef uint16_t millis_t;

/*
 * Halt program execution for `ms` milliseconds.
 * This method is not safe to use in an ISR or while
 * interrupts are disabled!
 */
void delay(millis_t ms);

/**
 * Initialize ACLK, setting it up to use the 32 kHz crystal.
 */
void aclk_init();

/**
 * Initialize SMCLK to be 16 MHz.
 */
void smclk_init();

/**
 * Intialize the timer and counters required for millis() and delay()
 * to function properly.
 */
void delay_init();

/**
 * Get a monotonic counter that increments about once every millisecond.
 */
millis_t millis();

#ifdef __cplusplus
}
#endif
#endif /* TARGETCONFIGS_TIMER_HPP_ */
