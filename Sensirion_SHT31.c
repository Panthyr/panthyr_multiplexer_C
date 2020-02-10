#include <xc.h>
#include "I2C1.h"
#define    FCY    6000000UL    // Instruction cycle frequency, Hz - required for __delayXXX() to work
#include <libpic30.h>          // For delay functions
#include "Sensirion_SHT31.h"

const char SHT31_Address = 0x45;
// constants with commands are in format:
// {int number of bytes, byte1, byte2}
const char CMD_SoftReset[3] = {2, 0x30, 0xA2};
const char CMD_SingleShot[3] = {2, 0x2C, 0x06};




int SHT31_InitReset(void){
    // check if device is on bus and responding
    if(I2C1_PollDevice(SHT31_Address) < 0){
        return -1;
    }
    
    // send soft reset command, this resets all config and loads calibration
    if(I2C1_Write(  SHT31_Address,\
                    0xFF,\
                    (char*)&CMD_SoftReset[1],\
                    CMD_SoftReset[0]\
                    ) <0){
        return -1;
    }else{
        return 0;
    }
}

int SHT31_SingleShot(signed int *buffer){
    unsigned char MeasurementData[6];
    
    // address device and start measurement     // intermediary buffer for data
    if(I2C1_Write(SHT31_Address,
                0xFF,
                &CMD_SingleShot[1],
                (char*)CMD_SingleShot[0])
                < 0){
        return -1;
    }
    
    // read back data to buffer
    __delay_ms(100);
    if(I2C1_Read(SHT31_Address,
                0xFF,
                &MeasurementData,
                6) < 0){
        return -1;
    }
    
    // check CRCs on data
    
    // calculate temp and RH
    SHT31_ConvertData(&MeasurementData, &buffer);
    
    return 0;
}

int SHT31_ConvertData(unsigned char *MeasurementData, signed int *buffer){
    unsigned int TempRaw,RHRaw = {0};
    char AsciiResult[5] = {0};
    double TempCelcius = 0;
    
    // combine both bytes to get raw temperature value
    TempRaw = ((int)MeasurementData[0] << 8);
    TempRaw += MeasurementData[1];
    
    itoa(&AsciiResult, TempRaw, 10);
    SendString(4, "Raw temp counts: ");
    SendString(4, &AsciiResult);
    
    TempCelcius = TempRaw;
    TempCelcius *= 175;
    TempCelcius /= 0xFFFF;
    TempCelcius -= 45;
    
    itoa(&AsciiResult, TempCelcius *100, 10);
    SendString(4, "\nDegrees Celcius (*100): ");
    SendString(4, &AsciiResult);
    SendString(4,"\n\n");
    
    
    return 0;
}

