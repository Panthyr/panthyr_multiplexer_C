/* 
 * File:   utils.h
 * Author: dieterv
 * 
 * Created on March 23, 2020, 5:04 PM
 */

#ifndef UTILS_H
#define	UTILS_H

#define _ISR __attribute__((interrupt, no_auto_psv))

// low level, used in intToStr
void reverse(char* str, int len);
// low level, used in ftoa
int intToStr(uint16_t x, char str[], int d, uint8_t neg);
// high level function
void ftoa(float n, char* res, uint8_t afterpoint);


#endif	/* UTILS_H */

