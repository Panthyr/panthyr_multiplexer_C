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

// FillString(): helper function
// Takes a (null-terminated) string and 
// lengthens it in-place to a given length with filler.
// In-place result is null-terminated.
// String gets right-justified if leftOrRight == 1, left-justified if -1
// source should be big enough to contain requested length
// First checks length of current string, returns 0 if longer than targetLength
// If right-justified: moves current string to end, then fills rest with filler
// If left-justified: adds filler char to (length -1) and ends with null-terminator
// Returns: 
// 1 if successful
// 0 if input string is longer than targetLength or incorrect leftOrRight value
uint16_t fillString(char * source, uint16_t targetLength, char filler, int8_t leftOrRight);

// ftoaFix() Combines ftoa and fillString to convert to a string with fixed length.
// Empty space to the left or right (depending on leftOrRigth) of the result is filled with filler
// Result gets right-justified in buf if leftOrRight == 1, left-justified if -1
// Returns:
// 1 if successful
// 0 if ftoa result is longer than targetLength
uint8_t ftoaFix(float floatToConvert, 
                char* buf, 
                uint8_t afterpoint, 
                uint16_t targetLength, 
                char filler, 
                int8_t leftOrRight);

// ftoaFixR() shorthand function for ftoaFix with fixed parameters:
// Right justified (leftOrRight = 1)
// Filler is a whitespace ' ' (0x20)
// Returns:
// 1 if successful
// 0 if ftoa result is longer than targetLength
uint8_t ftoaFixR(float floatToConvert, 
                char* buf, 
                uint8_t afterpoint, 
                uint16_t targetLength);

// ftoaFixL() shorthand function for ftoaFix with fixed parameters:
// Left justified (leftOrRight = -1)
// Filler is a whitespace ' ' (0x20)
// Returns:
// 1 if successful
// 0 if ftoa result is longer than targetLength
uint8_t ftoaFixL(float floatToConvert, 
                char* buf, 
                uint8_t afterpoint, 
                uint16_t targetLength);
#endif	/* UTILS_H */

