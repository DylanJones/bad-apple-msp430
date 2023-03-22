/*
 * defines.h
 *
 *  Created on: Mar 18, 2023
 *      Author: dylan
 */

#ifndef DEFINES_H_
#define DEFINES_H_

typedef unsigned int size_t;
#ifndef __cplusplus
typedef unsigned char bool;
#endif

#define BIC(reg, bit) reg &= ~(bit);
#define BIS(reg, bit) reg |= (bit);

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))


#endif /* DEFINES_H_ */
