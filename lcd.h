/*--------------------------------------------------------
GEORGE MASON UNIVERSITY
  Header file for routines for the built in LCD.

Date:   Fall 2022
Author: Jens-Peter Kaps
--------------------------------------------------------*/
#ifndef LCD_H_
#define LCD_H_

#include "msp430.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

int lcd_init();
void lcd_clear();
void displayNum(unsigned int);
unsigned int Dec2BCD(unsigned int);

//LCD Memory locations
#define pos1 9   /* Digit A1 begins at S18 */
#define pos2 5   /* Digit A2 begins at S10 */
#define pos3 3   /* Digit A3 begins at S6  */
#define pos4 18  /* Digit A4 begins at S36 */
#define pos5 14  /* Digit A5 begins at S28 */
#define pos6 7   /* Digit A6 begins at S14 */

extern const char digit[10][2];

#ifdef __cplusplus
}
#endif
#endif /* LCD_H_ */
