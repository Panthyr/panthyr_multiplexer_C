/*
 * File:   uart.h for PIC24FJ128GB204
 * Author: dieterv
 * v0.2
 */


// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef UART_H
#define	UART_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "hardware.h"
#include <time.h>

extern char AsciiResult[5];

void Uart_SendString( unsigned int port, char * buffer );
void Uart_SendStringFancy( unsigned int port, char *buffer );
void Uart_SendRaw( unsigned int port, char *buffer, unsigned int length );
void Uart_SendChar( unsigned int port, char ch);

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

