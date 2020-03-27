/* 
 * File:   Hypermaq/Panthyr serial port multiplex
 * Author: Dieter Vansteenwegen
 * Comments:
 * Revision history: 
 * v0.3
 * v0.4     Added communication/protocol between local and remote
 *          Pragma's moved to config.h
 *          Circular buffers restructured
 *          Variables declared as volatile when required
 *          Removed Uart Tx ISR's (and disabled interrupts)
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef MAIN_H
#define	MAIN_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdint.h>
#include "LSM9DS1.h"

const char FW_VERSION[5] = "v0.4";

/* METHODS */
void muxRad(void);
void muxIrr(void);
void sendVersion(void);
uint8_t getVitals(void);
void formatVitals(char PrintoutTemp[], char PrintoutHum[]);
void outputMuxedMsg(uint8_t TargetPort, uint16_t MsgLength, uint16_t MsgStartPos);
void processMuxedCmd(uint16_t MsgLength, uint16_t MsgStartPos);
void muxSendCommand();

uint8_t printIMUData(uint8_t port, 
                        imu_t * imu, 
                        bool printGyro, 
                        bool printAcc, 
                        bool printMag, 
                        bool printPR,
                        bool printHeading,
                        bool printAlsoIfOld);
uint8_t printGyro(uint8_t serialPort, imu_t * imu);
uint8_t printAcc(uint8_t serialPort, imu_t * imu);
uint8_t printMag(uint8_t serialPort, imu_t * imu);
uint8_t calcPitchRoll (imu_t * imu, float * pitch, float * roll);
uint8_t printPitchRoll(uint8_t serialPort, float * pPitch, float * pRoll);
uint8_t calcHeading(imu_t * imu, int16_t * pHeading);
uint8_t printHeading(uint8_t serialPort, int16_t * pHeading);

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

