/*--------------------------------------------------------
GEORGE MASON UNIVERSITY
  Routines for the built in LCD using efficient BCD conversion
  Note: LCD needs ACLK

Date:   Fall 2022
Author: Jens-Peter Kaps
--------------------------------------------------------*/
#include <lcd.h>

// LCD memory map for numeric digits
const char digit[10][2] =
{
    {0xFC, 0x28},  /* "0" LCD segments a+b+c+d+e+f+k+q */
    {0x60, 0x20},  /* "1" */
    {0xDB, 0x00},  /* "2" */
    {0xF3, 0x00},  /* "3" */
    {0x67, 0x00},  /* "4" */
    {0xB7, 0x00},  /* "5" */
    {0xBF, 0x00},  /* "6" */
    {0xE4, 0x00},  /* "7" */
    {0xFF, 0x00},  /* "8" */
    {0xF7, 0x00}   /* "9" */
};


const char digitpos[6] = {pos6, pos5, pos4, pos3, pos2, pos1};

void displayNum(unsigned int num) // only 4 digits
{
    unsigned int bcd;
    unsigned short i;
    unsigned int c;
    char numstart = 0;

    // hacc: clear
    lcd_clear();

    bcd = Dec2BCD(num);

    for(i=4; i>0; i--){
        c = bcd & 0xF000;
        c = c >> 12;
        bcd = bcd << 4;
        if(c == 0 && i == 1){
            numstart = 1;
        }
        if(!(c == 0 && numstart == 0 )){
            LCDMEM[digitpos[i-1]] = digit[c][0];
            LCDMEM[digitpos[i-1]+1] = digit[c][1];
            numstart = 1;
        }
    }
}

int lcd_init()
{
//    PJSEL0 = BIT4 | BIT5;                   // For LFXT

    LCDCPCTL0 = 0xFFD0;		// Init. LCD segments 4, 6-15
    LCDCPCTL1 = 0xF83F;		// Init. LCD segments 16-21, 27-31
    LCDCPCTL2 = 0x00F8; 	// Init. LCD segments 35-39

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Configure LFXT 32kHz crystal
    // Removed, because it's in a separate function now
//    CSCTL0_H = CSKEY >> 8;                  // Unlock CS registers
//    CSCTL4 &= ~LFXTOFF;                     // Enable LFXT
//    do
//    {
//      CSCTL5 &= ~LFXTOFFG;                  // Clear LFXT fault flag
//      SFRIFG1 &= ~OFIFG;
//    }while (SFRIFG1 & OFIFG);               // Test oscillator fault flag
//    CSCTL0_H = 0;                           // Lock CS registers

    // Initialize LCD_C
    // ACLK, Divider = 1, Pre-divider = 16; 4-pin MUX
    LCDCCTL0 = LCDDIV__1 | LCDPRE__16 | LCD4MUX | LCDLP | LCDSON;

    // VLCD generated internally,
    // V2-V4 generated internally, v5 to ground
    // Set VLCD voltage to 2.60v
    // Enable charge pump and select internal reference for it
    LCDCVCTL = VLCD_1 | VLCDREF_0 | LCDCPEN;

    LCDCCPCTL = LCDCPCLKSYNC;               // Clock synchronization enabled

    LCDCMEMCTL = LCDCLRM;                   // Clear LCD memory

    LCDCCTL0 |= LCDON;

	return 0;
}

void lcd_clear()
{
	// Initially, clear all displays.
	LCDCMEMCTL |= LCDCLRM;
}

// This function implements an efficient decimal to binary conversion.
// Note that a potential BCD overflow is not handled. In case this
// is needed, the function's return value as well as the data type
// of "Output" need to be changed from "unsigned int" to
// "unsigned long" and the intrinsics to __bcd_add_long(...).
unsigned int Dec2BCD(unsigned int Value)
{
    unsigned int i;
    unsigned int Output;
    for (i = 16, Output = 0; i; i--)
    // BCD Conversion, 16-Bit
    {
        Output = __bcd_add_short(Output, Output);
        if (Value & 0x8000)
            Output = __bcd_add_short(Output, 1);
        Value <<= 1;
    }
    return Output;
}
