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

/* METHODS */
void Uart_SendString( unsigned int port, const char * buffer );
void Uart_SendStringNL( unsigned int port, const char * buffer );
void Uart_SendRaw( unsigned int port, const char * buffer, unsigned int length );
void Uart_SendChar( unsigned int port, const char ch);

#endif	/* XC_HEADER_TEMPLATE_H */

