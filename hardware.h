/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef HARDWARE_H
#define	HARDWARE_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdbool.h>
#include <stdint.h>
#include "uart.h"

#define U1_Tx_SetDigOut()           _TRISB7=0
#define U1_Tx_SetHighOut()          _LATB7=1
#define U1_Rx_SetDigIn()            _TRISB8=1

#define U2_Tx_SetDigOut()           _TRISC8=0
#define U2_Tx_SetHighOut()          _LATC8=1
#define U2_Rx_SetDig()              _ANSB13=0
#define U2_Rx_SetDigIn()            _TRISB13=1

#define U3_Tx_SetDigOut()           _TRISC3=0
#define U3_Tx_SetHighOut()          _LATC3=1
#define U3_Rx_SetDigIn()            _TRISC4=1


/***********/
/* METHODS */
/***********/
void initHardware( void ); 
void initPPS( void );
void initOscillator( void );
void initInterrupts (void);
void InitU1(void);
void InitU2(void);
void InitU3(void);
void initTimer1( void );
void initTimer4( void );



/***********/
/* DEFINES */
/***********/
#define    INPUT_PIN        1
#define    OUTPUT_PIN       0

#define    DIGITAL          0
#define    ANALOG           1

/* TP */
#define TP32_SetHigh()          _LATA8 = 1
#define TP32_SetLow()           _LATA8 = 0
#define TP32_Toggle()           _LATA8 ^= 1
#define TP32_SetDigOut()        _TRISA8 = 0

#define TP35_SetHigh()          _LATA9 = 1
#define TP35_SetLow()           _LATA9 = 0
#define TP35_Toggle()           _LATA9 ^= 1
#define TP35_SetDigOut()        _TRISA9 = 0

/* LEDS */

#define LED4                    _LATB2
#define LED4_SetHigh()          _LATB2 = 1
#define LED4_SetLow()           _LATB2 = 0
#define LED4_Toggle()           _LATB2 ^= 1
#define LED4_SetDigOut()        _TRISB2 = 0

#define LED5                    _LATB3
#define LED5_SetHigh()          _LATB3 = 1
#define LED5_SetLow()           _LATB3 = 0
#define LED5_Toggle()           _LATB3 ^= 1
#define LED5_SetDigOut()        _TRISB3 = 0

#define LED6                    _LATA10
#define LED6_SetHigh()          _LATA10 = 1
#define LED6_SetLow()           _LATA10 = 0
#define LED6_Toggle()           _LATA10 ^= 1
#define LED6_SetDigOut()        _TRISA10 = 0

#define LED7                    _LATC5
#define LED7_SetHigh()          _LATC5 = 1
#define LED7_SetLow()           _LATC5 = 0
#define LED7_Toggle()           _LATC5 ^= 1
#define LED7_SetDigOut()        _TRISC5 = 0


typedef unsigned int            UINT;
typedef unsigned char           UINT8;
typedef unsigned short int      UINT16;
typedef unsigned char           BYTE;                           /* 8-bit unsigned  */
typedef unsigned short int      WORD;                           /* 16-bit unsigned */
typedef unsigned long           DWORD;                          /* 32-bit unsigned */

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

