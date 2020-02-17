/*
 * File:   uart.c for PIC24FJ128GB204
 * Author: dieterv
 * v0.2
 */

#include <xc.h>
#include "uart.h"

void Uart_SendString( unsigned int port, char *buffer ){
    /* Sends string at *buffer until 0x00 char, not adding any other char
     */
    switch(port){
        case 1:
            do{
                while( U1STAbits.UTXBF );       // Wait while Transmit buffer is full
                U1TXREG = *buffer;
            }while( *++buffer != 0x00);            // 0x00 is end of string
            while( !U1STAbits.TRMT );           // Transmit Shift Register is not empty and the transmit buffer is not empty
            break;

        case 2:
            do{
                while( U2STAbits.UTXBF );       // Wait while Transmit buffer is full
                U2TXREG = *buffer;
            }while( *++buffer != 0x00);            // 0x00 is end of string
            while( !U2STAbits.TRMT );           // Transmit Shift Register is not empty or the transmit buffer is not empty
            break;
        
        case 3:
            do{
                while( U3STAbits.UTXBF );       // Wait while Transmit buffer is full
                U3TXREG = *buffer;
            }while( *++buffer != 0x00);            // 0x00 is end of string
            while( !U3STAbits.TRMT );           // Transmit Shift Register is not empty or the transmit buffer is not empty
            break;
        
        case 4:
            do{
                while(U4STAbits.UTXBF);
                U4TXREG = *buffer;
            }while(*++buffer != 0x00);
            while( !U4STAbits.TRMT );
            break;

        default:
            break;

    }
}

void Uart_SendStringFancy( unsigned int port, char *buffer ){
    /* Sends string at *buffer, prefixed by "__Ux__" (where x = port) to UART port
     */
    unsigned int a = strlen(buffer) + 11;        // Count length of string, add 8 for "__Ux__"
    char msg[a];                                // Create room for complete message
    msg[0] = 0;                                 // Add null to first byte
    strcpy(msg, "__Ux__");                      // Add template to empty buffer
    msg[3] = '0' + port;                        // Add port number (offset port from ascii char '0')
    
    unsigned int i = 0;                         // index to array      
    strcat(msg, buffer);
    switch(port){
        
        case 1:
//            strcat(msg, "***\r\n");
            do{
                while( U1STAbits.UTXBF );       // Wait while Transmit buffer is full
                U1TXREG = msg[i];
                i++;
            }while( msg[i] != 0x00);            // 0x00 is end of string
            while( !U1STAbits.TRMT );           // Transmit Shift Register is not empty and the transmit buffer is not empty
            break;
        case 2:
            do{
                while( U2STAbits.UTXBF );       // Wait while Transmit buffer is full
                U2TXREG = msg[i];
                i++;
            }while( msg[i] != 0x00);            // 0x00 is end of string
            while( !U2STAbits.TRMT );           // Transmit Shift Register is not empty or the transmit buffer is not empty
            break;
        
        case 3:
            do{
                while( U3STAbits.UTXBF );       // Wait while Transmit buffer is full
                U3TXREG = msg[i];
                i++;
            }while( msg[i] != 0x00);            // 0x00 is end of string
            while( !U3STAbits.TRMT );           // Transmit Shift Register is not empty or the transmit buffer is not empty
            break;
        default:
            break;
    }
}

void Uart_SendRaw( unsigned int port, char *buffer, unsigned int length ){ 
    char target[10]={"("};
    char suffix[2]={")"};
    utoa(target+1, length, 10);     // Add to target at position +1
    
    switch(port){
        case 1:
            Uart_SendString(1, strcat(target,suffix));
            while(length != 0){
               while( U1STAbits.UTXBF );    /* Wait while Transmit buffer is full */
               U1TXREG = *buffer;
               buffer++;
               length--;
            }
            while( !U1STAbits.TRMT );   /* Transmit Shift Register is not empty and the transmit buffer is not empty */
            break;
        case 2:
            Uart_SendString(2, strcat(target,suffix));
            while(length != 0){
               while( U2STAbits.UTXBF );    /* Wait while Transmit buffer is full */
               U2TXREG = *buffer;
               buffer++;
               length--;
            }
            while( !U2STAbits.TRMT );   /* Transmit Shift Register is not empty and the transmit buffer is not empty */
            break;
        case 3:
            Uart_SendString(3, strcat(target,suffix));
            while(length != 0){
               while( U3STAbits.UTXBF );    /* Wait while Transmit buffer is full */
               U3TXREG = *buffer;
               buffer++;
               length--;
            }
            while( !U3STAbits.TRMT );   /* Transmit Shift Register is not empty and the transmit buffer is not empty */
            break;
        case 4:
            Uart_SendString(4, strcat(target,suffix));
            while(length != 0){
               while( U4STAbits.UTXBF );    /* Wait while Transmit buffer is full */
               U4TXREG = *buffer;
               buffer++;
               length--;
            }
            while( !U3STAbits.TRMT );   /* Transmit Shift Register is not empty and the transmit buffer is not empty */
            break;
        default:
            break;
    }
}

void Uart_SendChar( unsigned int port, char ch){ 
    switch(port){
        case 1:
            while( U1STAbits.UTXBF );    /* Wait while Transmit buffer is full */
            U1TXREG = ch;
            while( !U1STAbits.TRMT );   /* Transmit Shift Register is not empty and the transmit buffer is not empty */
            break;
        case 2:
           while( U2STAbits.UTXBF );    /* Wait while Transmit buffer is full */
           U2TXREG = ch;
            while( !U2STAbits.TRMT );   /* Transmit Shift Register is not empty and the transmit buffer is not empty */
            break;
        case 3:
            while( U3STAbits.UTXBF );    /* Wait while Transmit buffer is full */
            U3TXREG = ch;
            while( !U3STAbits.TRMT );   /* Transmit Shift Register is not empty and the transmit buffer is not empty */
            break;
        case 4:
            while( U4STAbits.UTXBF );    /* Wait while Transmit buffer is full */
            U4TXREG = ch;
            while( !U4STAbits.TRMT );   /* Transmit Shift Register is not empty and the transmit buffer is not empty */
            break;
        default:
            break;
    }
}