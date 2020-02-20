/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef MAIN_H
#define	MAIN_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdint.h>


/* METHODS */

void Print_Data(int16_t *pTemp, uint8_t *pRH);

/* Specify an extension for GCC based compilers */
#if defined(__GNUC__)
#define __EXTENSION __extension__
#else
#define __EXTENSION
#endif

#if !defined(__PACKED)
    #define __PACKED
#endif

//typedef union
//{
//    BYTE Val;
//    struct __PACKED
//    {
//        __EXTENSION BYTE b0:1;
//        __EXTENSION BYTE b1:1;
//        __EXTENSION BYTE b2:1;
//        __EXTENSION BYTE b3:1;
//        __EXTENSION BYTE b4:1;
//        __EXTENSION BYTE b5:1;
//        __EXTENSION BYTE b6:1;
//        __EXTENSION BYTE b7:1;
//    } bits;
//} BYTE_VAL, BYTE_BITS;

#endif	/* XC_HEADER_TEMPLATE_H */

