/*
 * File:   Sensirion_SHT31.c
 * Author: dieterv
 * v0.2
 * Created on February 3, 2020, 3:09 PM
 */

#include <xc.h>
#define    FCY    6000000UL    // Instruction cycle frequency, Hz - required for __delayXXX() to work
#include <libpic30.h>          // For delay functions
#include "Sensirion_SHT31.h"
#include "stdint.h"
#include "I2C1.h"

// Constants (address and commands):
const uint8_t SHT31_Address = 0x45;
// constants with commands are in format:
// {int number of bytes, byte1, byte2}
const uint8_t CMD_SoftReset[3] = {1, 0x30, 0xA2};
const uint8_t CMD_SSHighRep[3] = {1, 0x2C, 0x06};
const uint8_t CMD_SSMidRep[3] = {1, 0x2C, 0x0D};
const uint8_t CMD_SSLowRep[3] = {1, 0x2C, 0x10};
const uint8_t CMD_HeaterOn[3] = {1, 0x30, 0x6D};
const uint8_t CMD_HeaterOff[3] = {1, 0x30, 0x66};




int8_t SHT31_InitReset(void){
    // check if device is on bus and responding
    if(I2C1_PollDevice(SHT31_Address) < 0){
        return -1;
    }
    
    // send soft reset command, this resets all config and loads calibration
    if(I2C1_Write(  SHT31_Address,\
                    CMD_SoftReset[1],\
                    (uint8_t *)&CMD_SoftReset[2],\
                    CMD_SoftReset[0]\
                    ) <0){
        return -1;
    }else{
        return 0;
    }
}

int8_t SHT31_SingleShot(int16_t * pTempResult, uint8_t * pRHResult, uint8_t Repeatability){
    // High level function
    // Reads out sensor, checks both CRC's, 
    // Saves (temp *100) in *pTempResult, (int16_t)
    // Saves HR in *pRHResult (uint8_t)
    // Returns 1 if all went well
    // Returns 0 if not all went well
    uint8_t MeasurementData[6] = {0};           // buffer for the raw measurement data
    
    if (SHT31_GetData(MeasurementData, Repeatability) < 1){
        return -1;
    }
    // check CRCs on data
    if (SHT31_CheckCRC(MeasurementData) < 1){            // Temp CRC
        return -2;                                       // error checking CRC
    }
    if (SHT31_CheckCRC(&MeasurementData[3] ) < 1){       // RH CRC
        return -2;                                       // error checking CRC
    }
    
    // calculate temp and RH
    if (SHT31_ConvertData(MeasurementData, pTempResult, pRHResult) < 1){
        return -3;
    }

    return 0;                                           // all went well
}

int8_t SHT31_HeaterOnOff(uint8_t OnOff){
    // Switch heater on if OnOff = 1 / off if OnOff = 0
    // Returns:
    // 1 if success
    // -1 if failed or invalid option

    switch(OnOff){
        case 0:                     // Switch OFF
            if (I2C1_Write(SHT31_Address,
                    CMD_HeaterOff[1],
                    &CMD_HeaterOff[2],
                    CMD_HeaterOff[0])
                    < 0) {
                return -1;
            }
            break;
        case 1:                     // Switch ON
            if (I2C1_Write(SHT31_Address,
                            CMD_HeaterOn[1],
                            &CMD_HeaterOn[2],
                            CMD_HeaterOn[0])
                            < 0) {
                return -1;        
            }
            break;
        default:
            return -1;              // Invalid option
    }
    return 1;
}

int8_t SHT31_GetData(uint8_t MeasurementData[], uint8_t Repeatability){
    const uint8_t * pCommand;
    uint8_t MeasurementDuration;
    switch(Repeatability){
        case 1:                                 // High repeatability
            MeasurementDuration = 5;
            pCommand = &CMD_SSLowRep[0];
            break;
        case 2:                                 // High repeatability
            MeasurementDuration = 7;
            pCommand = &CMD_SSMidRep[0];
            break;
        case 3:                                 // High repeatability
            MeasurementDuration = 16;
            pCommand = &CMD_SSHighRep[0];
            break;
    }
       
    // address device and start measurement
    if(I2C1_Write(SHT31_Address,
                *(pCommand+1), // value at second field (is register)
                pCommand+2, // pointer + 2 is where payload starts
                *pCommand) // value at pointer (number of bytes to send)
                < 0){
        return -1;
    }
    
    // read back data to buffer
    __delay_ms(MeasurementDuration +2);    
    if(I2C1_Read(SHT31_Address,
                0xFF,
                MeasurementData,
                6) < 0){
        return -1;
    }
    return 1;
}

int8_t SHT31_CheckCRC(uint8_t DataToCheck[]){
    // Check CRC for RH and temp
    // CRC is one byte for two bytes of data
    // Provide pointer to first byte.
    // Second value and CRC are expected to follow directly after the first byte
    // Initial value: 0xFF
    // Polynomial: 0x31 (x^8 + x^5 + x^4 + 1)
    // returns 1 if correct
    // returns 0 if incorrect or issues
    
    const uint8_t POLYNOMIAL = 0x31;  // 
    uint8_t crc = 0xFF;               // initial value
    uint8_t c = 2;                    // fixed to two bytes input
    uint8_t i = 8;                    // for 8 bits in each byte
    
    for(c=2; c>0; c--){
        crc ^= (int)*DataToCheck++;              // after the second byte, the pointer is incremented
                                            // and points to the CRC value that we want to check
        for (i = 8; i>0;i--){
            if (crc & 0x80){
                crc = (crc << 1) ^ POLYNOMIAL;
            }else{
                crc = crc << 1;
            }
        }
    }
    if(crc == *DataToCheck){                    // story checks out
        return 1;
    }else{                                      // CRC mismatch
        return 0;
    }
}

int8_t SHT31_ConvertData(  uint8_t MeasurementData[],
                        int16_t * pTempResult, 
                        uint8_t * pRHResult){
    // converts data from raw counts to temp (deg C) and RH (%)
    
//    __attribute__((aligned(2))) double TempCelcius = 0;
    double TempCelcius = 0;
    uint16_t TempRaw,RHRaw = {0};
    double RH = 0;
    
//    combine both bytes to get raw temperature value
//    TempRaw = (MeasurementData[0] << 8);
//    TempRaw += MeasurementData[1];
    TempRaw = (MeasurementData[0] << 8) | (MeasurementData[1] & 0xff);
    TempCelcius = TempRaw;
    TempCelcius /= 0xFFFF;
    TempCelcius *= 175;
    TempCelcius -= 45;
    TempCelcius *= 100;
    *pTempResult = (int)TempCelcius;
    
//    Combine both bytes to get raw relative humidity value
//    RHRaw = (MeasurementData[3] << 8);
//    RHRaw += MeasurementData[4];
    RHRaw = (MeasurementData[3] << 8) | (MeasurementData[4] & 0xff);
    RH = RHRaw;
    RH /= 0xFFFF;
    RH *= 100;
    *pRHResult = RH;
    
    return 1;
}
