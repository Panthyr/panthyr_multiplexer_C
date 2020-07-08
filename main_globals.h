/* 
 * File:   main_globals.h
 * Author: dieterv
 *
 * Created on June 15, 2020, 3:30 PM
 */

#ifndef MAIN_GLOBALS_H
#define	MAIN_GLOBALS_H

#define COMMANDMAXLENGTH 50
#define BUFFLENGTH 1024

//struct CircBuf { // incoming data buffers
//    volatile uint8_t Buff[BUFFLENGTH];
//    volatile uint16_t WritePos; // Write position
//    volatile uint16_t ReadPos; // Read position
//    volatile uint16_t FillLength; // Number of unprocessed chars in array
//    volatile bool DoMux; // Flag if there's buffered RX from Ux
//} RadBuf, IrrBuf = {
//    {0, 0, 0, 0, 0}
//};

struct CircBuf { // incoming data buffers
    volatile uint8_t Buff[BUFFLENGTH];
    volatile uint16_t WritePos; // Write position
    volatile uint16_t ReadPos; // Read position
    volatile uint16_t FillLength; // Number of unprocessed chars in array
    volatile bool DoMux; // Flag if there's buffered RX from Ux
};
extern struct CircBuf RadBuf, IrrBuf;



struct MuxRxBuff {
    volatile uint8_t CircBuff[BUFFLENGTH]; // Circular buffer for UART3
    volatile uint16_t WritePos; // Write position
    volatile uint16_t MsgStart; // Read position
    volatile uint16_t MsgLength; // Number of unprocessed chars in array
    volatile uint8_t TargetPort; // Where should the message go to?
    volatile uint16_t ExpectedChr; // Counter for number of expected chars
    volatile uint8_t Preamble; // Counter for preamble
};

extern struct MuxRxBuff MuxRxBuff;

struct DemuxBuff { // describes the data to be demuxed in the MuxRxBuff
    volatile uint8_t TargetPort; // Where this message should go
    volatile uint16_t MsgLength; // Number of received chars in message
    // next var: position in the MuxRxBuff where the message starts
    volatile uint16_t MsgStartPos; 
};

extern struct DemuxBuff DeMuxBuffDescr;


extern volatile bool FlagMuxDoDemux;
extern volatile char AuxRx[COMMANDMAXLENGTH];
extern volatile bool FlagMuxDoDemux;  // Flag if there's buffered RX from U3
// next one is 1 if requested from aux serial, 2 if through mux
extern volatile uint8_t FlagVitalsRequested; 
extern volatile uint8_t FlagVersionRequested; // send version info to aux
// next one is 1 if requested from aux serial, 2 if through mux
extern volatile uint8_t FlagImuRequested;
extern volatile uint8_t FlagImuCalib;
#endif	/* MAIN_GLOBALS_H */

