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
//#include "uart.h"
//#include "Sensirion_SHT31.h"
//#include "I2C1.h"


//typedef unsigned int            UINT;
//typedef unsigned char           UINT8;
//typedef unsigned short int      UINT16;
//typedef unsigned char           BYTE;                           /* 8-bit unsigned  */
//typedef unsigned short int      WORD;                           /* 16-bit unsigned */
//typedef unsigned long           DWORD;                          /* 32-bit unsigned */


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
void InitU4(void);
void initTimer1( void );
void initTimer4( void );

/***********/
/* DEFINES */
/***********/

/* GENERIC STUFF */
#define    HIGH             1
#define    LOW              0
#define    SET              1
#define    CLEAR            0

#define U1_Tx_SetDigOut()           _TRISC7=0
#define U1_Tx_SetHighOut()          _LATC7=1
#define U1_Rx_SetDigIn()            _TRISC6=1

#define U2_Tx_SetDigOut()           _TRISC8=0
#define U2_Tx_SetHighOut()          _LATC8=1
#define U2_Rx_SetDig()              _ANSB13=0
#define U2_Rx_SetDigIn()            _TRISB13=1

#define U3_Tx_SetDigOut()           _TRISB7=0
#define U3_Tx_SetHighOut()          _LATB7=1
#define U3_Rx_SetDigIn()            _TRISC9=1

#define U4_Tx_SetDigOut()           _TRISC3=0
#define U4_Tx_SetHighOut()          _LATC3=1
#define U4_Rx_SetDig()              _ANSC4=0
#define U4_Rx_SetDigIn()            _TRISC4=1

#define    INPUT_PIN        1
#define    OUTPUT_PIN       0
#define    DIGITAL          0
#define    ANALOG           1

/* TestPoints */
#define TP19_SetHigh()          _LATA0 = 1
#define TP19_SetLow()           _LATA0 = 0
#define TP19_Toggle()           _LATA0 ^= 1
#define TP19_SetDig()           _ANSA0 = 0
#define TP19_SetDigOut()        _TRISA0 = 0

//TP32 is also attached to TOP/BOTTOM Switch
#define TP32_SetHigh()          _LATA8 = 1
#define TP32_SetLow()           _LATA8 = 0
#define TP32_Toggle()           _LATA8 ^= 1
#define TP32_SetDigOut()        _TRISA8 = 0

#define TP35_SetHigh()          _LATA9 = 1
#define TP35_SetLow()           _LATA9 = 0
#define TP35_Toggle()           _LATA9 ^= 1
#define TP35_SetDigOut()        _TRISA9 = 0

/* LEDS */

#define LED_Sens_Rx                    _LATB2
#define LED_Sens_Rx_SetHigh()          _LATB2 = 1
#define LED_Sens_Rx_SetLow()           _LATB2 = 0
#define LED_Sens_Rx_Toggle()           _LATB2 ^= 1
#define LED_Sens_Rx_SetDigOut()        _TRISB2 = 0

#define LED_Mux_Rx                      _LATB3
#define LED_Mux_Rx_SetHigh()            _LATB3 = 1
#define LED_Mux_Rx_SetLow()             _LATB3 = 0
#define LED_Mux_Rx_Toggle()             _LATB3 ^= 1
#define LED_Mux_Rx_SetDigOut()          _TRISB3 = 0

#define LED_Heartbeat                    _LATA10
#define LED_Heartbeat_SetHigh()          _LATA10 = 1
#define LED_Heartbeat_SetLow()           _LATA10 = 0
#define LED_Heartbeat_Toggle()           _LATA10 ^= 1
#define LED_Heartbeat_SetDigOut()        _TRISA10 = 0

#define LED_Boot                        _LATC5
#define LED_Boot_SetHigh()              _LATC5 = 1
#define LED_Boot_SetLow()               _LATC5 = 0
#define LED_Boot_Toggle()               _LATC5 ^= 1
#define LED_Boot_SetDigOut()            _TRISC5 = 0

/* Top/bottom switch */
#define TopBotSwitch                    _LATA8
#define TopBotSwitch_SetDigIn()         _TRISA8 = 1


#define StartWDT()                  RCONbits.SWDTEN = 0x01;    // Start Software WDT (512ms for PRE = POSTscaler = 128)

#endif	/* XC_HEADER_TEMPLATE_H */

