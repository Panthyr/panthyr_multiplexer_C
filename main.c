/*
 * Hypermaq/Panthyr serial port multiplex
 * SW version v0.5
 * For HW TOP v0.2, Bottom v0.2
 * v0.1 of mux board has only 3 serial ports. The serial port that is labeled "mux"
 * is connected to the pins that connect to uart4 on newer hardware...
 *
 *  * v0.4     Added communication/protocol between local and remote
 *          Pragma's moved to config.h
 *          Circular buffers restructured
 *          Variables declared as volatile when required
 *          Removed Uart Tx ISR's (and disabled interrupts)
 * v0.5     Moved all interrupts to interrupts.c and globals to main_globals.h
 */

/* <> vs "" for includes:
   #include <x.h> --> first look among the compiler standard header files
   #include "x.h" --> first look in the program directory
 */
#define FCY 6000000UL    // Instruction cycle frequency, Hz - required for __delayXXX() to work
#include <libpic30.h>               // contains delay functions
#include "config.h"
#include "main.h"
#include "string.h"                 // for strcat
#include <stdlib.h>                 // for itoa
#include "hardware.h"               // 
#include "uart.h"
#include "I2C1.h"
#include "Sensirion_SHT31.h"        // temp/RH sensor
#include "LSM9DS1.h"        // IMU
#include "utils.h"
#include "main_globals.h"   // declarations for global variables (used in interrupts and main)

// Variables

/* Variables/constants for the UARTS */

const char FW_VERSION[5] = "v0.5";

volatile char AuxRx[COMMANDMAXLENGTH] = {0}; // Buffer for the received commands on UART4

/* FLAGS */
volatile bool FlagMuxDoDemux = 0; // Flag if there's buffered RX from U3
// next one is 1 if requested from aux serial, 2 if through mux
volatile uint8_t FlagVitalsRequested = 0; 
volatile uint8_t FlagVersionRequested = 0; // send version info to aux
volatile uint8_t FlagImuCalib = 0;
// next one is 1 if requested from aux serial, 2 if through mux
volatile uint8_t FlagImuRequested = 0;
// next variable to identify that we've requested data from "the other side"
volatile bool FlagWaitingForRemoteVitals = 0; 
bool FlagTxCMDMux = 0; // to flag that we need to send a command over the mux

/* WORKING VARIABLES */
int16_t SHT31_Temp = 0;
uint8_t SHT31_RH = 0;
// buffer for command that needs to be send to remote
char CmdToMux[COMMANDMAXLENGTH] = {0}; 
//uint16_t MsgNumber = 0; // identifies requests to remote mux/demux board




/* IMU structs */
imu_config_t imuconfig;
imu_t imu;



void muxSendCommand()
{
    uint8_t CmdLength = 0;
    uint8_t CmdPos = 0;
    // first count number of characters 
    while (CmdToMux[CmdPos++] != 0x00) {
        CmdLength++;
    }
    // reset position so we can start sending
    CmdPos = 0;

    Uart_SendString(3, "_(0"); // Send prefix
    Uart_SendChar(3, CmdLength); // Send number of chars as one byte
    Uart_SendString(3, ")_");
    while (CmdLength-- > 0) {
        Uart_SendChar(3, CmdToMux[CmdPos++]);
    }
    Uart_SendChar(3, 13); // Send CR
    memset(CmdToMux, 0, COMMANDMAXLENGTH); // clear command buffer
    FlagTxCMDMux = 0;
}

void muxRad(void)
{
    /* Make working copies of the fill length and read position, 
 They can change if new data comes in during transmit. */
    uint16_t UxFillLengthCopy = 0; // will hold length of message in buffer
    uint16_t UxRead = 0; // read position in mux

    UxFillLengthCopy = RadBuf.FillLength;
    UxRead = RadBuf.ReadPos;
    RadBuf.ReadPos = RadBuf.WritePos;
    RadBuf.FillLength = 0;
    Uart_SendString(3, "_(1"); // Send prefix
    Uart_SendChar(3, UxFillLengthCopy); // Send number of chars as one byte
    Uart_SendString(3, ")_");
    while (UxFillLengthCopy > 0) {
        Uart_SendChar(3, RadBuf.Buff[UxRead]);
        UxRead++;
        if (UxRead == BUFFLENGTH) {
            UxRead = 0;
        }
        UxFillLengthCopy--;
    }
    Uart_SendChar(3, 13); // Send CR
}

void muxIrr(void)
{
    /* Make working copies of the fill length and read position, 
    They can change if new data comes in during transmit. */
    unsigned int UxFillLengthCopy = 0; // will hold length of message in buffer
    unsigned int UxRead = 0; // read position in mux

    UxFillLengthCopy = IrrBuf.FillLength;
    UxRead = IrrBuf.ReadPos;
    IrrBuf.FillLength = 0;
    IrrBuf.ReadPos = IrrBuf.WritePos;
    Uart_SendString(3, "_(2"); // Send prefix
    Uart_SendChar(3, UxFillLengthCopy); // Send number of chars as one byte
    Uart_SendString(3, ")_");
    while (UxFillLengthCopy > 0) {
        Uart_SendChar(3, IrrBuf.Buff[UxRead]);
        UxRead++;
        if (UxRead == BUFFLENGTH) {
            UxRead = 0;
        }
        UxFillLengthCopy--;
    }
    Uart_SendChar(3, 13); // Send CR
}

void getVersion(void)
{
    Uart_SendString(4, "FW Version: ");
    Uart_SendStringNL(4, FW_VERSION); // end with newline
    // TODO: remote version
}

uint8_t getVitals(void)
{
    char PrintoutTemp[13] = {};
    char PrintoutHum[6] = {};

    if (FlagVitalsRequested == 1) { // vitals requested from aux serial port (local)
        // first task should be to ask other side for vitals
        memcpy(CmdToMux, "?vitals*", 9);
        FlagTxCMDMux = 1;
        FlagWaitingForRemoteVitals = 1;
    }

    // get the local values
    SHT31_SingleShot(&SHT31_Temp, &SHT31_RH, 3);
    // create message, regardless of where it should be sent to
    formatVitals(PrintoutTemp, PrintoutHum);

    if (FlagVitalsRequested == 1) { // local request, send data to local port
        Uart_SendStringNL(4, PrintoutTemp);
    }
    if (FlagVitalsRequested == 2) { // remote request, send data to remote (mux)
        // queue data to send to mux
        char TempReply[15] = "r";
        strcat(TempReply, PrintoutTemp);
        strcat(TempReply, "*");
        memcpy(CmdToMux, TempReply, sizeof (TempReply));
        FlagTxCMDMux = 1;
    }
    return 1;
}

void formatVitals(char PrintoutTemp[], char PrintoutHum[])
{
    if (ImBottom) {
        strcpy(PrintoutTemp, "tb");
        strcpy(PrintoutHum, ",hb");
    } else { // I'm top!
        strcpy(PrintoutTemp, "tt");
        strcpy(PrintoutHum, ",ht");
    }
    itoa(&PrintoutTemp[2], SHT31_Temp, 10);
    itoa(&PrintoutHum[3], SHT31_RH, 10);
    strcat(PrintoutTemp, PrintoutHum);
}

void getImu (void)
{   
    char imuData[26] = {0};
    if (ImTop){
        // we need to get the imu data, regardless of where the req comes from
        fillImuData(imuData);
    }
    if (FlagImuRequested == 1 && ImTop){
        Uart_SendString(4,imuData);
        // finished here
        return;
    }
    if (FlagImuRequested == 1 && ImBottom){
        //send imuData request to remote
        
        return;
    }
    if (FlagImuRequested == 2){
        // request comes from other station and that one is bottom
        if (ImBottom){
            // bad situation, both are set to bottom
            // prepare string with blank data
            strcpy(imuData, "p:---\nr:---\nh:---\n");
        }
        // if ImTop, imuData has already been populated
        // queue data to send to mux
        char TempReply[15] = "r";
        strcat(TempReply, imuData);
        strcat(TempReply, "*");
        memcpy(CmdToMux, TempReply, sizeof (TempReply));
        FlagTxCMDMux = 1;
        return;
    }
}

void calibImu (void)
{
    if (ImBottom){
        Uart_SendStringNL(4, "Cannot calibrate IMU, configured as bottom board.");
        return;
    }
    imuconfig.calibrate = 1;
    if (LSM9DS1_init(&imu, &imuconfig)){
        Uart_SendStringNL(4, "IMU calibration successful");
    }else{
        Uart_SendStringNL(4, "Error during IMU calibration. Check connections and try again.");
    }
    return;
}

void fillImuData (char * imuData){
        // will store the message in format p:xxx.yy\nr:xxx.yy\nh:xxx/n
        float PitchValue, RollValue = 0;
        float * pRollValue = &RollValue;
        float * pPitchValue = &PitchValue;
        int16_t Heading = 999;
        int16_t * pHeading = &Heading;
        calcPitchRoll (&imu, pPitchValue, pRollValue);
        calcHeading(&imu, pHeading);
        // pitch
        char temp[8] = {0};
        strcat(imuData,"p:");
        ftoa(PitchValue, temp, 2);
        strcat(imuData, temp);
        strcat(imuData,"\n");
        // roll
        temp[0] = 0;
        strcat(imuData,"r:");
        ftoa(RollValue, temp, 2);
        strcat(imuData, temp);
        strcat(imuData,"\n");
        // heading
        temp[0] = 0;
        strcat(imuData,"h:");
        itoa(temp,Heading,10);
        strcat(imuData, temp);
        strcat(imuData,"\n");
}

void outputMuxedMsg(uint8_t TargetPort, uint16_t MsgLength, uint16_t MsgStartPos)
{
    while (MsgLength-- > 0) {
        Uart_SendChar(TargetPort, MuxRxBuff.CircBuff[MsgStartPos++]);
        if (MsgStartPos == BUFFLENGTH) {
            MsgStartPos = 0;
        }
    }
}

void processMuxedCmd(uint16_t MsgLength, uint16_t MsgStartPos)
{
    // handles messages that target this board
    char ProcCommand[COMMANDMAXLENGTH] = {0}; // create buffer
    uint8_t ProcCommandPos = 0; // start at zero
    uint16_t i = MsgLength;
    while (i-- > 0) { // copy data to buffer
        ProcCommand[ProcCommandPos++] = MuxRxBuff.CircBuff[MsgStartPos++];
        if (MsgStartPos == BUFFLENGTH) { // go back to beginning of circular buffer
            MsgStartPos = 0;
        }
    }

    if ((ProcCommand[0] == 'r')&&(ProcCommand[MsgLength - 1] == '*')) {
        // this is a reply (starting with r, ending with *)
        if (FlagWaitingForRemoteVitals) {
            // TODO: send received data out on mux
            uint8_t i = 1;
            while (i < MsgLength - 1) { // don't send the last character (*))
                Uart_SendChar(4, ProcCommand[i++]);
            }
            Uart_SendChar(4, '\n');
            FlagWaitingForRemoteVitals = 0;
        }
        // add possible FlagWaitingForXXX flags
        return;
    }
    
    // check for known commands
    if (strcmp(ProcCommand, "?vitals*") == 0) {
        FlagVitalsRequested = 2;
    }
    if (strcmp(ProcCommand, "?imu*") == 0) {
        FlagImuRequested = 2;
    }
}

uint8_t printIMUData(uint8_t port, 
                        imu_t * imu, 
                        bool doPrintGyro, 
                        bool doPrintAcc, 
                        bool doPrintMag, 
                        bool doPrintPitchRoll,
                        bool doPrintHeading,
                        bool printAlsoIfOld)
{   
    bool printedSomething = 0;
    // gyro (and pitch/roll)
    if ((printAlsoIfOld) || (LSM9DS1_gyroAvailable(imu))){
        if (doPrintGyro){
            printGyro(port, imu);
            printedSomething = 1;
        }
        ClrWdt();
        if (doPrintPitchRoll){
            // prepare pointers to floats to store results
            float PitchValue, RollValue = 0;
            float * pRollValue = &RollValue;
            float * pPitchValue = &PitchValue;

            calcPitchRoll (imu, pPitchValue, pRollValue);
            printPitchRoll(port, pPitchValue, pRollValue);
            printedSomething = 1;
        }
    }
    if (doPrintAcc){
        if ((printAlsoIfOld) || (LSM9DS1_accelAvailable(imu))){ 
            printAcc(port, imu);
            printedSomething = 1;
        }
    }
    if ((printAlsoIfOld) || (LSM9DS1_magAvailable(imu))){
        if (doPrintMag){
            printMag(port,imu);
            printedSomething = 1;
        }
        if (doPrintHeading){
            int16_t Heading = 361;
            int16_t * pHeading = &Heading;
            calcHeading(imu, pHeading);
            printHeading(port, pHeading);
            printedSomething = 1;
        }
    }
    
    if (printedSomething){
        return 1;
    }else{
        return 0;
    }
}

uint8_t printGyro(uint8_t serialPort, imu_t * imu){
    float x, y, z;
    char print[10] = {0};
    
    // get data
    x = LSM9DS1_calcGyro(imu, imu->gx);
    y = LSM9DS1_calcGyro(imu, imu->gy);
    z = LSM9DS1_calcGyro(imu, imu->gz);
    
    // format and print to port
    Uart_SendString(serialPort, "G:");
    ftoa(x, print, 4);
    Uart_SendString(serialPort, print);
    Uart_SendChar(serialPort, ',');
    ftoa(y, print, 4);
    Uart_SendString(serialPort, print);
    Uart_SendChar(serialPort, ',');
    ftoa(z, print, 4);
    Uart_SendStringNL(serialPort, print);
    
    return 1;
}

uint8_t printAcc(uint8_t serialPort, imu_t * imu){
    float x, y, z;
    char print[10] = {0};
    
    // get data
    LSM9DS1_readAccel(imu);
    x = LSM9DS1_calcAccel(imu, imu->ax);
    y = LSM9DS1_calcAccel(imu, imu->ay);
    z = LSM9DS1_calcAccel(imu, imu->az);
    
    // convert and print
    Uart_SendString(serialPort, "A:");
    ftoa(x, print, 4);
    Uart_SendString(serialPort, print);
    Uart_SendChar(4, ',');
    ftoa(y, print, 4);
    Uart_SendString(serialPort, print);
    Uart_SendChar(4, ',');
    ftoa(z, print, 4);
    Uart_SendStringNL(serialPort, print);
    
    return 1;
}

uint8_t printMag(uint8_t serialPort, imu_t * imu){
    float x, y, z;
    char print[10] = {0};
    
    // get data
    LSM9DS1_readMag(imu);
    x = LSM9DS1_calcMag(imu, imu->mx);
    y = LSM9DS1_calcMag(imu, imu->my);
    z = LSM9DS1_calcMag(imu, imu->mz);
    
    // convert and print
    Uart_SendString(serialPort, "M:");
    ftoa(x, print, 4);
    Uart_SendString(serialPort, print);
    Uart_SendChar(4, ',');
    ftoa(y, print, 4);
    Uart_SendString(serialPort, print);
    Uart_SendChar(4, ',');
    ftoa(z, print, 4);
    Uart_SendStringNL(serialPort, print);

    return 1;
}

uint8_t printPitchRoll(uint8_t serialPort, float * pPitch, float * pRoll)
{
    char print[10] = {0};
    
    Uart_SendString(serialPort,"P:");
    ftoa(*pPitch, print,2);
    Uart_SendStringNL(serialPort, print);
    Uart_SendString(serialPort,"R:");
    ftoa(*pRoll, print, 2);
    Uart_SendStringNL(serialPort, print);
    return 1;
}

uint8_t printHeading(uint8_t serialPort, int16_t * pHeading)
{
    char print[10] = {0};
    
    Uart_SendString(serialPort,"H:");
    itoa(print, *pHeading, 10);
    Uart_SendStringNL(serialPort, print);
    return 1;
}


int main(void)
{

    /*Initialize*/
    LED_Boot_SetHigh(); // After startup, light red led for 1 second (100 PWM cycles)
    initHardware(); // Init all the hardware components and setup pins

    
    
    if(ImTop){
        imuconfig.calibrate = 0;
        imuconfig.enable_accel = 1;
        imuconfig.enable_gyro = 1;
        imuconfig.enable_mag = 1;
        imuconfig.low_power_mode = 0;
        if (LSM9DS1_init(&imu, &imuconfig)){
            Uart_SendStringNL(4, "IMU init done.");
        }else{
            Uart_SendStringNL(4, "NO CONNECTION TO IMU");
        }
    }
    StartWDT();
 
     /*Main loop*/
    while (1) {
        ClrWdt(); // kick wdt
        
        // check if request have been received over mux or aux serial
        if (FlagVitalsRequested > 0) {
            getVitals();
            FlagVitalsRequested = 0; //
        }
        // check if version information has been requested
        if (FlagVersionRequested) {
            getVersion();
            FlagVersionRequested = 0;
        }
        // check if imu data from top has been requested
        if (FlagImuRequested > 0){
            getImu();
            FlagImuRequested = 0;
        }
        
        if (FlagImuCalib) { // calibrate the IMU (should be leveled)
            calibImu();
            FlagImuCalib = 0;
        }

        if (RadBuf.DoMux) { // send data from Radiance over mux
            muxRad();
            RadBuf.DoMux = false; // flag might be a new one but we don't care
        }

        if (IrrBuf.DoMux) { // send data from Irr over mux
            muxIrr();
            IrrBuf.DoMux = false; // flag might be a new one but we don't care
        }

        if (FlagTxCMDMux) { // need to send a command over mux
            muxSendCommand();
        }
        
        if (FlagMuxDoDemux) { // a message has come in
            FlagMuxDoDemux = 0; // must be before the loop to avoid race condition:
            /* If a complete new frame comes in during the while loop below,
               and then MuxDoDemux gets set to 0 at the end,
               if no new tmr4 interrupt gets called, 
             * that buffer part never gets written out */

            // now handle the package
            if (DeMuxBuffDescr.TargetPort == 0) { 
                // msg has to be handled by processor
                processMuxedCmd(DeMuxBuffDescr.MsgLength, 
                                DeMuxBuffDescr.MsgStartPos);
            } else {
                outputMuxedMsg(DeMuxBuffDescr.TargetPort, 
                                DeMuxBuffDescr.MsgLength, 
                                DeMuxBuffDescr.MsgStartPos);
            }
        }
        __delay_ms(50);
    }
}