/* 
 * File:   utils.h
 * Author: dieterv
 * 
 * Created on March 23, 2020, 5:04 PM
 */

#ifndef UTILS_H
#define	UTILS_H

#undef _ISR
#define __ISR __attribute__((interrupt, no_auto_psv))

// low level, used in intToStr
void reverse(char* str, int len);
//intToStr() low level, used in ftoa
// Converts a given integer value to string str[].  
// minChar is the number of digits required in the output.  
// If minChar is more than the number of digits in str[],  
// then 0s are added at the beginning. 
// for use with ftoa())
// returns int: number of characters used
int intToStr(uint16_t x, char str[], int d, uint8_t neg);
// ftoa(): high level function
// Checks if float floatToConvert is pos/neg, remembers sign and works with absolute value
// Splits float floatToConvert in integerPart and floatRemainder
// Converts integerPart with sign to string, gets number of characters as return
// Returns number of chars used (including zero termination)
uint16_t ftoa(float floatToConvert, char* buf, uint8_t afterpoint);


#endif	/* UTILS_H */

