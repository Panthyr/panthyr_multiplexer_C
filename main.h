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
#define BUFFLENGTH 1024

const char FW_VERSION[5] = "v0.4";

struct CircBuf { // incoming data buffers
    volatile uint8_t Buff[BUFFLENGTH];
    volatile uint16_t WritePos; // Write position
    volatile uint16_t ReadPos; // Read position
    volatile uint16_t FillLength; // Number of unprocessed chars in array
    volatile bool DoMux; // Flag if there's buffered RX from Ux
} RadBuf, IrrBuf = {
    {0, 0, 0, 0, 0}
};

struct MuxRxBuff {
    volatile uint8_t CircBuff[BUFFLENGTH]; // Circular buffer for UART3
    volatile uint16_t WritePos; // Write position
    volatile uint16_t MsgStart; // Read position
    volatile uint16_t MsgLength; // Number of unprocessed chars in array
    volatile uint8_t TargetPort; // Where should the message go to?
    volatile uint16_t ExpectedChr; // Counter for number of expected chars
    volatile uint8_t Preamble; // Counter for preamble
} MuxRxBuff = {
    {0}, 0, 0, 0, 0, 0, 0
};

struct DemuxBuff { // describes the data to be demuxed in the MuxRxBuff
    volatile uint8_t TargetPort; // Where this message should go
    volatile uint16_t MsgLength; // Number of received chars in message
    // next var: position in the MuxRxBuff where the message starts
    volatile uint16_t MsgStartPos; 
} DeMuxBuffDescr = {0, 0, 0};

/* METHODS */
void muxRad(void);
void muxIrr(void);
void getVersion(void);
// flagVitalsRequested can be 1 (requested locally) 
// or 2(requested remotely over mux)
// get local temp/rh, send them out of the mux (if requested remotely) or 
// uart4 (if requested locally)
// if requested locally, send request over mux and set flag to wait for it
uint8_t getVitals(void);
void formatVitals(char PrintoutTemp[], char PrintoutHum[]);
// flagImuRequested can be 1 (requested locally) 
// or 2(requested remotely over mux)
// if flagImuRequested==1 and I'm top -> send p/r/h data over uart4 (aux)
// if flagImuRequested==1 and not top -> send request over mux
// if flagImuRequested==2 and I'm top -> send p/r/h data over mux
// if flagImuRequested==2 and not top -> send p/r/h with --- as value
void getImu (void);
// calibImu() re-inits the IMU, with calibration enabled
void calibImu (void);
void fillImuData (char * imuData);
void outputMuxedMsg(uint8_t TargetPort, uint16_t MsgLength, uint16_t MsgStartPos);
// processMuxedCmd() processMuxedCmd handles incoming (from mux) commands:
// - It checks the validity of the message format.
// - If  it is a known command, sets the corresponding flag.
// - If the message is a response to a request sent by itself (starts with "r"), 
// it checks to see if there are any "waiting for reply"flags set. 
// If one is set, it handles the message (output on UART4/AUX in case of ?vitals*) 
// and clears the flag.
void processMuxedCmd(uint16_t MsgLength, uint16_t MsgStartPos);
void muxSendCommand();
// printIMUData() - gets, formats and prints out data from IMU
// Mostly for debugging and testing
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
uint8_t printPitchRoll(uint8_t serialPort, float * pPitch, float * pRoll);
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

