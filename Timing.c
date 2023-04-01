/*
 * Timer.cpp
 *
 *  Created on: Mar 1, 2023
 *      Author: dylan
 */

#include <Timing.h>
#include <stdbool.h>

volatile static bool msTriggered = false;

/*****
 * API functions
 *****/

void aclk_init() {
    PJSEL0 = BIT4 | BIT5;                   // For LFXT
    // Configure LFXT 32kHz crystal
    CSCTL0_H = CSKEY >> 8;                  // Unlock CS registers
    CSCTL4 &= ~LFXTOFF;                     // Enable LFXT
    do
    {
      CSCTL5 &= ~LFXTOFFG;                  // Clear LFXT fault flag
      SFRIFG1 &= ~OFIFG;
    }while (SFRIFG1 & OFIFG);               // Test oscillator fault flag
    CSCTL0_H = 0;                           // Lock CS registers
}

void smclk_init() {
    // high speed system (16 MHz MCLK)
    FRCTL0 = FRCTLPW | NWAITS_1; // unlock FRCTL, enable wait states
    FRCTL0_H = 0; // lock FRCTL
    CSCTL0 = CSKEY; // unlock CSCTL
    CSCTL1 = DCORSEL | DCOFSEL_4; // very HF cpu
                                  // my ISRs are fast now 😈
    CSCTL2 = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = 0;
    CSCTL0_H = 0; // lock CSCTL
}

void delay_init() {
    // Configure TA3 for delay() and millis() usage.
    TA3CCTL0 = 0;
    TA3CCR0 = 0;
    TA3CTL = TASSEL__ACLK + ID__8 + TACLR + MC__CONTINOUS;
    TA3EX0 = 4; // 8 * 4 == prescaler of 32, for ~1024 Hz clock
}

millis_t millis() {
    // Instead of a full Booyer-Moore majority vote, we can apparently just wait for
    // two stable readings in a row.
    int reading = TA3R;
    while (TA3R != reading) reading = TA3R;
    return reading;
}


void delay(millis_t ms) {
    // Stop timer
    TA3CTL = 0;
    // Schedule interrupt for X ticks in the future
    TA3CCR0 = TA3R + ms;
    msTriggered = false;
    // Make sure we're being interrupted when we need to be
    TA3CCTL0 = CCIE;
    TA3CTL = TASSEL__ACLK + ID__8 + MC__CONTINOUS;
    while (!msTriggered)
        __low_power_mode_1();
    TA3CCTL0 = 0; // turn interrupt back off
}

/*****
 * Interrupt Handlers
 *****/

/*
 * Timer A3 CCR0 interrupt
 */
#pragma vector=TIMER3_A0_VECTOR
__interrupt void TimerA3_CCR0_ISR() {
    // Do nothing except wake up from LPM (and set the flag).
    msTriggered = true;
    __low_power_mode_off_on_exit();
}
