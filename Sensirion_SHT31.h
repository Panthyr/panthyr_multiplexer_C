/*
 * File:   Sensirion_SHT31.h
 * Author: dieterv
 * v0.2
 * Created on February 3, 2020, 3:09 PM
 * 
 * Library for Sensirion SHT31 Temp/Relative humidity sensor
 * Usage:
 * - Set I2C address in Sensirion_SHT31.c file (const uint8_t SHT31_Address = 0x45)
 *      ! Default address with address pin tied to ground is 0x44
 * - Run SHT31_InitReset
 * - Run SHT31_SingleShot with pointers to where results should be stored and required repeatability
 * - Internal heater can be turned on/off with SHT31_HeaterOnOff: 1 to switch on, 0 to switch off
 */

#ifndef XC_SHT31
#define	XC_SHT31

#include <xc.h> // include processor files - each processor file is guarded.  
#include "I2C1.h"

//--------------------Variables--------------------
extern char AsciiResult[5];
//-------------------Macros-------------------

//--------------------Functions--------------------

int8_t SHT31_InitReset (void);                             // high level
int8_t SHT31_HeaterOnOff(uint8_t);                         // high level
int8_t SHT31_SingleShot(int16_t *, uint8_t *, uint8_t);    // high level
int8_t SHT31_GetData(uint8_t [], uint8_t Repeatability);
int8_t SHT31_CheckCRC(uint8_t []);
int8_t SHT31_ConvertData(uint8_t[], int16_t *, uint8_t *);
#endif

