/*
 * Hypermaq/Panthyr serial port multiplex
 * SW version v0.3
 * For HW TOP v0.2, Bottom v0.2
 * v0.1 of mux board has only 3 serial ports. The serial port that is labeled "mux"
 * is connected to the pins that connect to uart4 on newer hardware...
 * v0.3
 * v0.4     Added communication/protocol between local and remote
 *          Pragma's moved to config.h
 *          Circular buffers restructured
 *          Variables declared as volatile when required
 *          Removed Uart Tx ISR's (and disabled interrupts)
 */

/* <> vs "" for includes:
   #include <x.h> --> first look among the compiler standard header files
   #include "x.h" --> first look in the program directory
 */
#define FCY 6000000UL    // Instruction cycle frequency, Hz - required for __delayXXX() to work
#include "config.h"
#include "main.h"
//#include <stdbool.h>
#include "string.h"                 // for strcat
#include <stdlib.h>                 // for itoa
#include <libpic30.h>               // contains delay functions
#include "hardware.h"               // 
#include "uart.h"
#include "I2C1.h"
#include "Sensirion_SHT31.h"        // temp/RH sensor
#include "LSM9DS1.h"        // IMU
#include "utils.h"
#include "math.h"
#define PI 3.14159265359

// Variables

/* Variables/constants for the UARTS */
#define BUFFLENGTH 1024
#define COMMANDMAXLENGTH 50

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
    volatile uint16_t MsgStartPos; // position in the MuxRxBuff where the message starts
} DeMuxBuffDescr = {0, 0, 0};

char AuxRx[COMMANDMAXLENGTH] = {0}; // Buffer for the received commands on UART4
volatile uint8_t AuxRxPos = 0; // Write position in the buffer

/* FLAGS */
volatile bool FlagMuxDoDemux = 0; // Flag if there's buffered RX from U3
volatile uint8_t FlagVitalsRequested = 0; // 1 if requested from aux serial, 2 if through mux
volatile uint8_t FlagVersionRequested = 0; // send version info to aux
volatile bool FlagWaitingForRemoteVitals = 0; // to identify that we've requested data from "the other side"
bool FlagTxCMDMux = 0; // to flag that we need to send a command over the mux

// WORKING VARIABLES
int16_t SHT31_Temp = 0;
uint8_t SHT31_RH = 0;
char CmdToMux[COMMANDMAXLENGTH] = {0}; // buffer for command that needs to be send to remote
//uint16_t MsgNumber = 0; // identifies requests to remote mux/demux board


/* Variables for the heartbeat LED
   PWM cycle is 936 steps, so full off - full on cycle takes
   (936/7)*0.01s = 0.85s */
bool LedState = 0; // Heartbeat led state
uint16_t PWMValue = 0x300; // Start value for PWM duty
int8_t PWMStep = 7; // Step size for PWM
bool CountUp = 1; // For the PWM of the heartbeat led
uint16_t bootLed = 500; // Used to light orange led for 2.5s after boot (400 half PWM 0.01s cycles)
// Decremented each half PWM duty cycle until zero

void __ISR _U1RXInterrupt(void)
{
    // radiance data
    IFS0bits.U1RXIF = false; // clear interrupt bit
    LED_Sens_Rx_Toggle();
    char x = U1RXREG;

    RadBuf.Buff[RadBuf.WritePos] = x;
    RadBuf.FillLength++;
    RadBuf.WritePos++;
    if (RadBuf.WritePos == BUFFLENGTH) {
        RadBuf.WritePos = 0;
    }
}

void __ISR _U2RXInterrupt(void)
{
    // irradiance data
    IFS1bits.U2RXIF = false; // clear interrupt bit
    LED_Sens_Rx_Toggle();
    char x = U2RXREG;

    IrrBuf.Buff[IrrBuf.WritePos] = x;
    IrrBuf.FillLength++;
    IrrBuf.WritePos++;
    if (IrrBuf.WritePos == BUFFLENGTH) {
        IrrBuf.WritePos = 0;
    }
}

void __ISR _U3RXInterrupt(void)
{
    // muxed data
    IFS5bits.U3RXIF = false; // clear interrupt bit
    LED_Mux_Rx_Toggle();
    unsigned char x = U3RXREG;
    switch (MuxRxBuff.Preamble) {
    case 0: // Waiting for start of muxed message
        if (x == '_') { // Correct start char
            MuxRxBuff.MsgStart = MuxRxBuff.WritePos; // "reserve" start position
            MuxRxBuff.Preamble++; // One char of preamble correct
        }
        break;
    case 1:
        if (x == '(') {
            MuxRxBuff.Preamble++;
        } else {
            MuxRxBuff.Preamble = 0; // Incorrect preamble, restart
        }
        break;
    case 2:
        if (x - 0x30 < 5) { // Port number should be 1,2 or 4 (4 can be temp request)
            MuxRxBuff.TargetPort = x - 0x30;
            MuxRxBuff.Preamble++;
        } else {
            MuxRxBuff.Preamble = 0;
        }
        break;
    case 3: // _(x was already received
        if (x > 0) {
            MuxRxBuff.ExpectedChr = x;
            MuxRxBuff.Preamble++;
        } else {
            MuxRxBuff.Preamble = 0;
        }
        break;
    case 4: // _(xy) has been received
        if (x == ')') {
            MuxRxBuff.Preamble++;
        } else {
            MuxRxBuff.Preamble = 0; // Incorrect preamble, restart
        }
        break;
    case 5:
        if (x == '_') { // _(xy)_ has been received
            MuxRxBuff.Preamble++;
            break;
        } else {
            MuxRxBuff.Preamble = 0; // Incorrect preamble, restart
        }
        break;
    case 6:
        if (MuxRxBuff.ExpectedChr > 0) { // Store data until all expected chars received
            MuxRxBuff.CircBuff[MuxRxBuff.WritePos] = x;
            MuxRxBuff.ExpectedChr--;
            MuxRxBuff.MsgLength++;
            MuxRxBuff.WritePos++;
            if (MuxRxBuff.WritePos == BUFFLENGTH) {
                MuxRxBuff.WritePos = 0;
            }
            break;
        } else { // complete message should have been received
            if (x == 13) { // check for CR
                // send data out if correctly terminated
                FlagMuxDoDemux = 1;
                MuxRxBuff.Preamble = 0;
                // set variables to describe the message to be demuxed
                DeMuxBuffDescr.TargetPort = MuxRxBuff.TargetPort;
                DeMuxBuffDescr.MsgLength = MuxRxBuff.MsgLength;
                DeMuxBuffDescr.MsgStartPos = MuxRxBuff.MsgStart;
            } else { // no CR when expected, dump message
                // or throw away and start over
                MuxRxBuff.Preamble = 0;
                MuxRxBuff.WritePos = MuxRxBuff.MsgStart;
            }
            MuxRxBuff.ExpectedChr = 0;
            MuxRxBuff.MsgLength = 0;
        }

    }
}

void __ISR _U4RXInterrupt(void)
{
    // this interrupt waits for a message start (either ? or !),
    // then buffers the command up to the * character
    // buffer is COMMANDMAXLENGTH bytes, max command is COMMANDMAXLENGTH - 2 (COMMANDMAXLENGTH - start ? or ! - end *)
    // if a known command is received, it sets its flag for the main loop to handle
    // if it gets more than COMMANDMAXLENGTH bytes, an unknown command or a new ?/! character,
    // it clears the buffer and restarts

    // possible commands:

    // ?vitals*:
    // command for requesting temp and RH is ?vitals* (all lowercase!)
    // replies are in the format rxxx,yyy,zzz*
    // each "field" is RH or temp*100 of top or bottom
    // temp top: ttxxxx, temp bottom: tbxxxx 
    // RH top: htxxx, RH bottom: hbxxx
    // if the board does not get a reply from the other side over the mux
    // within x (to be determined) amount of time, it only sends its own data

    IFS5bits.U4RXIF = false; // clear interrupt bit
    char x = U4RXREG;
    switch (x) {
    case '!': // beginning of new command
        memset(AuxRx, 0, sizeof (AuxRx)); // start new receive operation, clear previous content
        AuxRx[0] = '!';
        AuxRxPos = 1; // wait for the next character to come in
        break;
    case '?': // beginning of new request
        memset(AuxRx, 0, sizeof (AuxRx)); // start new receive operation, clear previous content
        AuxRx[0] = '?';
        AuxRxPos = 1; // wait for the next character to come in
        break;
    case '*': // end of command/request, check what has been requested
        AuxRx[AuxRxPos++] = '*'; // add the * to the buffer and set to next position
        AuxRx[AuxRxPos] = 0; // add 0 (should not be needed since array was initialized with zeros?)

        /* check for correct commands*/
        if (strcmp(AuxRx, "?vitals*") == 0) {
            FlagVitalsRequested = 1;
        }

        if (strcmp(AuxRx, "?ver*") == 0) {
            FlagVersionRequested = 1;
        }

        AuxRxPos = 0; // either a valid command was passed and the appropriate flag set,
        memset(AuxRx, 0, sizeof (AuxRx)); // or the command was invalid. Either way, restart receive
        break;
    default: // not one of the special characters we're looking for
        if (AuxRxPos > 0) { // should already have received ? or !
            AuxRx[AuxRxPos++] = x; // add character and set to next position
        }
        break;
    }

    if (AuxRxPos > COMMANDMAXLENGTH) { // too many chars to be valid, restart rx operation
        AuxRxPos = 0;
        memset(AuxRx, 0, sizeof (AuxRx));
        // clear buffer
    }
}

void __ISR _T1Interrupt()
{
    /* ISR for TIMER 1 
       Heartbeat LED_Heartbeat (orange) and bootup led (Red)
       0x3A8 is about 10ms (0.06% error)
       Higher PWMValue: Higher brightness
     */
    IFS0bits.T1IF = false; // clear interrupt flag
    TMR1 = 0x0000;
    if (LedState) { // Led is currently on
        LED_Heartbeat_SetLow(); // switch off
        PR1 = 0x3A8 - PWMValue; // TMR1 set for off time (10ms - on time)
    } else {
        LED_Heartbeat_SetHigh();
        PR1 = PWMValue;
        if (CountUp) {
            PWMValue = PWMValue + PWMStep;
        } else {
            PWMValue = PWMValue - PWMStep;
        }
        if (PWMValue > 0x100) {
            PWMStep = 15; // Value of PWMStep changes depending on duty cycle to compensate for non-linearity of led brightness
        } else {
            PWMStep = 2;
        }
        if (PWMValue > 0x390) {
            CountUp = 0; // count down again
        }
        if (PWMValue < 10) {
            CountUp = 1; // count up
        }
    }
    LedState = !LedState;
    /* Boot led shows that PIC has (re)started... 
     red LED_Boot starts counting down from 200, decrementing each half PWM cycle.
     if it is not 0, decrement 1
     if, after decrementing it is zero, switch off boot led
     */
    if (bootLed) {
        --bootLed; // First decrement counter
        if (!bootLed) { // If zero after decrement,
            LED_Boot_SetLow(); // Switch off
        }
    }
}

void __ISR _T4Interrupt()
{
    /* ISR for TIMER 4 
       Generate clock to send data to mux every 50ms
     */
    IFS1bits.T4IF = false; //clear interrupt flag
    TMR4 = 0x0000;
    if (RadBuf.FillLength > 0) {
        RadBuf.DoMux = true;
    }
    if (IrrBuf.FillLength > 0) {
        IrrBuf.DoMux = true;
    }
}

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

void sendVersion(void)
{
    Uart_SendString(4, "FW Version: ");
    Uart_SendStringNL(4, FW_VERSION); // end with newline
}

uint8_t getVitals(void)
{
    // flagVitalsRequested can be 1 (requested locally) or 2(requested remotely over mux)
    // get local temp/rh, send them out of the mux (if requested remotely) or 
    // uart4 (if requested locally)
    // if requested locally, send request over mux and set flag to wait for it
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
    }

    if (strcmp(ProcCommand, "?vitals*") == 0) {
        FlagVitalsRequested = 2;
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
    Uart_SendStringNL(4, "HW init done.");
    struct imu_config_s imuconfig;
    imuconfig.calibrate = 1;
    imuconfig.enable_accel = 1;
    imuconfig.enable_gyro = 1;
    imuconfig.enable_mag = 1;
    imuconfig.low_power_mode = 0;
    struct imu_s imu;

    LSM9DS1_init(&imu, &imuconfig);
    Uart_SendStringNL(4, "IMU init done.");

    StartWDT();
    
    /*Main loop*/
    while (1) {
        ClrWdt(); // kick wdt
        //        char temp[10] = {0};
        //        int16_t LSM_Temp = 0;
        //        LSM9_GetTemp(&LSM_Temp);
        //        itoa(temp, LSM_Temp, 10);
        //        Uart_SendStringNL(4, temp);
        
        printIMUData(4,&imu,1,1,1,1,1,0);
        

        // check if request have been received over mux or aux serial
        if (FlagVitalsRequested > 0) {
            getVitals();
            FlagVitalsRequested = 0; //
        }
        // check if version information has been requested
        if (FlagVersionRequested) {
            sendVersion();
            FlagVersionRequested = 0;
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
               if no new tmr4 interrupt gets called, that buffer part never gets written out */

            // now handle the package
            if (DeMuxBuffDescr.TargetPort == 0) { // msg has to be handled by processor
                processMuxedCmd(DeMuxBuffDescr.MsgLength, DeMuxBuffDescr.MsgStartPos);
            } else {
                outputMuxedMsg(DeMuxBuffDescr.TargetPort, DeMuxBuffDescr.MsgLength, DeMuxBuffDescr.MsgStartPos);
            }
        }
        __delay_ms(75);
    }
    return 0;
}
