#include <xc.h>
#include "ST_LSM9DS1.h"
#include "stdint.h"
#include "I2C1.h"
//#define    FCY    6000000UL    // Instruction cycle frequency, Hz - required for __delayXXX() to work
//#include <libpic30.h>          // For delay functions


const uint8_t LSM9_AG_Address = 0x6A;  // address for acc/gyro with pin SA0 pulled low
const uint8_t LSM9_MAG_Address = 0x1C;  // address for magneto with pin SA1 pulled low

// Registers for gyro and acc
const uint8_t CMD_REG1_G[3] = {1, 0x10, 0b00100000};    // enable gyro and set ODR
const uint8_t CMD_REG3_G[3] = {1, 0x12, 0b10000000};    // set LP mode and dis HPF
const uint8_t CMD_REG6_XL[3] = {1, 0x20, 0b00100000};    // EN ACC, ODR to 10Hz, FS +/-2G, BW by ODR (408Hz)
const uint8_t CMD_FIFO[3] = {1, 0x2E, 0b00100000};    // continuous mode
const uint8_t CMD_REG9[3] = {1, 0x23, 0b00000000};    // continuous mode


// Registers for mag
const uint8_t CMD_REG1_M[3] = {1, 0x20, 0b10000000};    // EN MAG, Temp_comp on, low power mode, ODR 0,625Hz, Self test off
const uint8_t CMD_REG3_M[3] = {1, 0x22, 0b11000000};    // I2C EN, Low power (0.625Hz), SPI write only, Single conversion

// Data registers
const uint8_t TEMP_H = 0x16;
const uint8_t TEMP_L = 0x15;


    
int8_t LSM9_InitReset (void)
{
    // ACC/Gyro
    if(I2C1_Write(  LSM9_AG_Address,\
                CMD_REG1_G[1],\
                (uint8_t *)&CMD_REG1_G[2],\
                CMD_REG1_G[0]\
                ) <0){
        return -1;
    } 
    
    if(I2C1_Write(  LSM9_AG_Address,\
                CMD_REG3_G[1],\
                (uint8_t *)&CMD_REG3_G[2],\
                CMD_REG3_G[0]\
                ) <0){
        return -1;
    } 
    
    if(I2C1_Write(  LSM9_AG_Address,\
                CMD_REG6_XL[1],\
                (uint8_t *)&CMD_REG6_XL[2],\
                CMD_REG6_XL[0]\
                ) <0){
        return -1;
    }
    
    if(I2C1_Write(  LSM9_AG_Address,\
                CMD_FIFO[1],\
                (uint8_t *)&CMD_FIFO[2],\
                CMD_FIFO[0]\
                ) <0){
        return -1;
    }
    
    // Mag sensor
    if(I2C1_Write(  LSM9_MAG_Address,\
                CMD_REG3_M[1],\
                (uint8_t *)&CMD_REG1_M[2],\
                CMD_REG3_M[0]\
                ) <0){
        return -1;
    }
    
    return 1;
}

int8_t LSM9_GetTemp (int16_t * pTempRaw)
{   
//    uint8_t TempHigh = 0, TempLow = 0;
    uint8_t TempGet[2]; 
    int32_t Temp =0;
    
//    if (I2C1_Read(LSM9_AG_Address, TEMP_H, &TempHigh, 1) < 1){
//        return 0;
//    }
//    if (I2C1_Read(LSM9_AG_Address, TEMP_L, &TempLow, 1) < 1){
//        return 0;
//    }
//    Temp = (((uint16_t)TempHigh << 8) | TempLow) &0xFFF;
    
    if (I2C1_Read(LSM9_AG_Address, TEMP_L, TempGet, 2) < 1){
        return 0;
    }
    if((Temp >> 11) == 0){  // positive number
        Temp = (Temp & 0x7FF); // keep bits [10:0]
    }else{
        Temp = ~Temp + 1; // invert bits, add one
        Temp = -(Temp & 0x7FF); // negative, and keep bits [10:0]
    }
    *pTempRaw = (int16_t)((Temp*100/16)+2500);  // everything *100 to avoid floating point
    return 1;
}