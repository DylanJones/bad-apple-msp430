/*
 * defines.h
 *
 * Miscellaneous defines / preprocessor macros to include in all files
 *
 *  Created on: Mar 18, 2023
 *      Author: dylan
 */

#ifndef DEFINES_H_
#define DEFINES_H_

// Helper to access the bit clear instruction
#define BIC(reg, bit) reg &= ~(bit);
// Helper to access the bit set instruction
#define BIS(reg, bit) reg |= (bit);

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))


#endif /* DEFINES_H_ */
