/*
 * Hypermaq/Panthyr serial port multiplex
 * SW version v0.3
 * For HW TOP v0.2, Bottom v0.2
 * v0.1 of mux board has only 3 serial ports. The serial port that is labeled "mux"
 * is connected to the pins that connect to uart4 on newer hardware...
 * v0.3
 */
// CONFIG1
#pragma config WDTPS = PS512    // Watchdog Timer Postscaler Select->1:512
#pragma config FWPSA = PR128    // WDT Prescaler Ratio Select->1:128
#pragma config WINDIS = OFF    // Windowed WDT Disable->Standard Watchdog Timer
#pragma config FWDTEN = SWON    // Watchdog Timer controlled by SWDTEN
#pragma config ICS = PGx1    // Emulator Pin Placement Select bits->Emulator functions are shared with PGEC1/PGED1
#pragma config LPCFG = OFF    // Low power regulator control->Disabled - regardless of RETEN
#pragma config GWRP = OFF    // General Segment Write Protect->Write to program memory allowed
#pragma config GCP = OFF    // General Segment Code Protect->Code protection is disabled
#pragma config JTAGEN = OFF    // JTAG Port Enable->Disabled

// CONFIG2
#pragma config POSCMD = NONE    // Primary Oscillator Select->Primary Oscillator Disabled
#pragma config WDTCLK = LPRC    // WDT Clock Source Select bits->WDT uses LPRC
#pragma config OSCIOFCN = ON    // OSCO Pin Configuration->OSCO/CLKO/RA3 functions as port I/O (RA3)
#pragma config FCKSM = CSECMD    // Clock Switching and Fail-Safe Clock Monitor Configuration bits->Clock switching is enabled, Fail-Safe Clock Monitor is disabled
#pragma config FNOSC = FRCPLL    // Initial Oscillator Select->Fast RC Oscillator with PLL module (FRCPLL)
#pragma config ALTRB6 = APPEND    // Alternate RB6 pin function enable bit->Append the RP6/ASCL1/PMPD6 functions of RB6 to RA1 pin functions
#pragma config ALTCMPI = CxINC_RB    // Alternate Comparator Input bit->C1INC is on RB13, C2INC is on RB9 and C3INC is on RA0
#pragma config WDTCMX = WDTCLK    // WDT Clock Source Select bits->WDT clock source is determined by the WDTCLK Configuration bits
#pragma config IESO = ON    // Internal External Switchover->Enabled

// CONFIG3
#pragma config WPFP = WPFP127    // Write Protection Flash Page Segment Boundary->Page 127 (0x1FC00)
#pragma config SOSCSEL = ON    // SOSC Selection bits->SOSC circuit selected
#pragma config WDTWIN = PS25_0    // Window Mode Watchdog Timer Window Width Select->Watch Dog Timer Window Width is 25 percent
#pragma config PLLSS = PLL_FRC    // PLL Secondary Selection Configuration bit->PLL is fed by the on-chip Fast RC (FRC) oscillator
#pragma config BOREN = ON    // Brown-out Reset Enable->Brown-out Reset Enable
#pragma config WPDIS = WPDIS    // Segment Write Protection Disable->Disabled
#pragma config WPCFG = WPCFGDIS    // Write Protect Configuration Page Select->Disabled
#pragma config WPEND = WPENDMEM    // Segment Write Protection End Page Select->Write Protect from WPFP to the last page of memory

// CONFIG4
#pragma config DSWDTPS = DSWDTPS1F    // Deep Sleep Watchdog Timer Postscale Select bits->1:68719476736 (25.7 Days)
#pragma config DSWDTOSC = LPRC    // DSWDT Reference Clock Select->DSWDT uses LPRC as reference clock
#pragma config DSBOREN = ON    // Deep Sleep BOR Enable bit->DSBOR Enabled
#pragma config DSWDTEN = ON    // Deep Sleep Watchdog Timer Enable->DSWDT Enabled
#pragma config DSSWEN = ON    // DSEN Bit Enable->Deep Sleep is controlled by the register bit DSEN
#pragma config PLLDIV = PLL6X    // USB 96 MHz PLL Prescaler Select bits->6x PLL selected
#pragma config I2C1SEL = DISABLE    // Alternate I2C1 enable bit->I2C1 uses SCL1 and SDA1 pins
#pragma config IOL1WAY = ON    // PPS IOLOCK Set Only Once Enable bit->Once set, the IOLOCK bit cannot be cleared

/* <> vs "" for includes:
   #include <x.h> --> first look among the compiler standard header files
   #include "x.h" --> first look in the program directory
 */
#define FCY 6000000UL    // Instruction cycle frequency, Hz - required for __delayXXX() to work
#include <xc.h>
#include "main.h"
//#include "stdint.h"                 // for typedefs
#include "string.h"                 // for strcat
#include <stdbool.h>
#include <stdlib.h>                 // itoa
#include <libpic30.h>    // contains delay functions
#include "hardware.h"               // 
#include "uart.h"
#include "I2C1.h"
#include "Sensirion_SHT31.h"        // temp/RH sensor

// Variables

/* Variables/constants for the UARTS */
#define BUFFLENGTH 1024
#define COMMANDMAXLENGTH 50
struct CircBuf{
    uint8_t Buff[BUFFLENGTH];
    uint16_t WritePos;           // Write position
    uint16_t ReadPos;            // Read position
    uint16_t FillLength;      // Number of unprocessed chars in array
    bool DoMux;                   // Flag if there's buffered RX from Ux
}RadBuf, IrrBuf = {{0,0,0,0,0}};

struct MuxRxBuff{
    uint8_t CircBuf[BUFFLENGTH];   // Circular buffer for UART3
    uint16_t WritePos;           // Write position
    uint16_t MessStart;            // Read position
//    unsigned int MuxFillLength = 0;      // Number of unprocessed chars in array
    uint8_t SourcePort;     // Where should the message go to?
    uint16_t ExpectedChr;    // Counter for number of expected chars
    uint8_t Preamble;       // Counter for preamble
}MuxRxBuff = {{0},0,0,0,0,0};

struct DeMuxBuff{
    uint8_t ToSendData[BUFFLENGTH]; // Message that needs to be send out
    uint8_t ToSendPort;         // Where this message should go
    uint16_t MuxMsgLength;       // Number of received chars in message
}DeMuxBuff = {{0},1,0};

char AuxRx[COMMANDMAXLENGTH] = {0}; // Buffer for the received commands on UART4
uint8_t AuxRxPos = 0;               // Write position in the buffer

/* FLAGS */
volatile bool flagMuxDoDemux = 0;                // Flag if there's buffered RX from U3
volatile uint8_t VitalsRequested = 0;        // 1 if requested from aux serial, 2 if through mux
volatile uint8_t flagVersionRequested = 0;        // 
volatile bool flagWaitingForRemoteVitals;       // to identify that we've requested data from "the other side"
signed int SHT31_Temp = 0;
unsigned char SHT31_RH = 0;

/* Variables for the heartbeat LED
   PWM cycle is 936 steps, so full off - full on cycle takes
   (936/7)*0.01s = 0.85s */
bool LedState = 0;              // Heartbeat led state
unsigned int PWMValue = 0x300;  // Start value for PWM duty
signed int PWMStep = 7;         // Step size for PWM
bool CountUp = 1;               // For the PWM of the heartbeat led
unsigned int bootLed = 500;     // Used to light orange led for 2.5s after boot (400 half PWM 0.01s cycles)
                                // Decremented each half PWM duty cycle until zero


void __attribute__ ( ( interrupt, no_auto_psv ) ) _DefaultInterrupt ( void ){ 
    Uart_SendString(1, "DefaultInterrupt");
    Uart_SendString(2, "DefaultInterrupt");
    Uart_SendString(3, "DefaultInterrupt");
    Uart_SendString(4, "DefaultInterrupt");
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U1TXInterrupt ( void ){ 
IFS0bits.U1TXIF = false;
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U1RXInterrupt ( void ){
    // radiance data
    IFS0bits.U1RXIF = false; // clear interrupt bit
    LED_Sens_Rx_Toggle();
    unsigned char x = U1RXREG;

    RadBuf.Buff[RadBuf.WritePos] = x;
    RadBuf.FillLength++;
    RadBuf.WritePos++;
    if (RadBuf.WritePos == BUFFLENGTH){
        RadBuf.WritePos = 0;
    }
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U2RXInterrupt ( void ){ 
    // irradiance data
    IFS1bits.U2RXIF = false; // clear interrupt bit
    LED_Sens_Rx_Toggle();
    unsigned char x = U2RXREG;
    IrrBuf.Buff[IrrBuf.WritePos] = x;
    IrrBuf.FillLength++;
    IrrBuf.WritePos++;
    if (IrrBuf.WritePos == BUFFLENGTH){
        IrrBuf.WritePos = 0;
    }
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U3RXInterrupt ( void ){ 
    // muxed data
    IFS5bits.U3RXIF = false; // clear interrupt bit
    LED_Mux_Rx_Toggle();
    unsigned char x = U3RXREG;
    switch(MuxRxBuff.Preamble){
        case 0:                             // Waiting for start of muxed message
            if (x == '_'){                  // Correct start char
                MuxRxBuff.MessStart = MuxRxBuff.WritePos;    // "reserve" start position
                MuxRxBuff.Preamble++;              // One char of preamble correct
            } 
            break;
        case 1:
            if (x == '('){
                MuxRxBuff.Preamble++;
            } else {
                MuxRxBuff.Preamble =0;         // Incorrect preamble, restart
            }
            break;
        case 2:
            if (x - 0x30 < 5){                     // Port number should be 1,2 or 4 (4 can be temp request)
                MuxRxBuff.SourcePort = x - 0x30;
                MuxRxBuff.Preamble++;
            } else {
                MuxRxBuff.Preamble = 0;
            }
            break;
        case 3:                             // _(x was already received
            if (x > 0){
                MuxRxBuff.ExpectedChr = x;
                MuxRxBuff.Preamble++;
            } else { 
                MuxRxBuff.Preamble = 0;
            }
            break;
        case 4:                             // _(x) has been received
            if (x == ')'){
                MuxRxBuff.Preamble++;
            } else {
                MuxRxBuff.Preamble =0;         // Incorrect preamble, restart
            }
            break;
        case 5:
            if (x == '_'){                  // _(x)_ has been received
                MuxRxBuff.Preamble++;
                break;
            } else {
                MuxRxBuff.Preamble =0;         // Incorrect preamble, restart
            }
            break;
        case 6:
            if (MuxRxBuff.ExpectedChr > 0){           // Store data until all expected chars received
                MuxRxBuff.CircBuf[MuxRxBuff.WritePos] = x;
                MuxRxBuff.ExpectedChr--;
                DeMuxBuff.MuxMsgLength++;
                MuxRxBuff.WritePos++;
                if (MuxRxBuff.WritePos == BUFFLENGTH){
                    MuxRxBuff.WritePos = 0;
                }
                break;
             } else if ( x == 13){
                //send data out
                flagMuxDoDemux = 1;
                MuxRxBuff.Preamble = 0;
             } else {                           // No CR when expected, dump message
                MuxRxBuff.Preamble =0;
             }
    }
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U4RXInterrupt ( void ){ 
    // this interrupt waits for a message start (either ? or !),
    // then buffers the command up to the * character
    // buffer is COMMANDMAXLENGTH bytes, max command is COMMANDMAXLENGTH - 2 (COMMANDMAXLENGTH - start ? or ! - end *)
    // if a known command is received, it sets its flag for the main loop to handle
    // if it gets more than COMMANDMAXLENGTH bytes, an unknown command or a new ?/! character,
    // it clears the buffer and restarts

    // possible commands:
    
    // ?vitals*:
    // command for requesting temp and RH is ?vitals* (all lowercase!)
    // replies are in the format !xxx,yyy,zzz*
    // each "field" is RH or temp*100 of top or bottom
    // temp top: ttxxxx, temp bottom: tbxxxx 
    // RH top: htxxx, RH bottom: hbxxx
    // if the board does not get a reply from the other side over the mux
    // within x (to be determined) amount of time, it only sends its own data
    
    IFS5bits.U4RXIF = false; // clear interrupt bit
    char x = U4RXREG;
    switch(x){
        case '!':           // beginning of new command
            memset(AuxRx, 0, sizeof(AuxRx));    // start new receive operation, clear previous content
            AuxRx[0] = '!';    
            AuxRxPos = 1;   // wait for the next character to come in
            break;
        case '?':           // beginning of new request
            memset(AuxRx, 0, sizeof(AuxRx));    // start new receive operation, clear previous content
            AuxRx[0] = '?';    
            AuxRxPos = 1;   // wait for the next character to come in
            break;
        case '*':           // end of command/request, check what has been requested
            AuxRx[AuxRxPos++] = '*';      // add the * to the buffer and set to next position
            AuxRx[AuxRxPos] = 0;        // add 0 (should not be needed since array was initialized with zeros?)
            
            /* check for correct commands*/
            if (strcmp( AuxRx, "?vitals*") == 0){
                VitalsRequested = 1;
            }
            
            if(strcmp (AuxRx, "?ver*") == 0){
                flagVersionRequested = 1;
            }
            
            AuxRxPos = 0;       // either a valid command was passed and the appropriate flag set,
            memset(AuxRx, 0, sizeof(AuxRx));        // or the command was invalid. Either way, restart receive
            break;
        default:                // not one of the special characters we're looking for
            if (AuxRxPos > 0){              // should already have received ? or !
                AuxRx[AuxRxPos++] = x;      // add character and set to next position
            }
            break;
    }
    
    if (AuxRxPos > COMMANDMAXLENGTH){         // too many chars to be valid, restart rx operation
        AuxRxPos = 0;
        memset(AuxRx, 0, sizeof(AuxRx));;   // clear buffer
    }
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _T1Interrupt (  ){
/* ISR for TIMER 1 
   Heartbeat LED_Heartbeat (orange) and bootup led (Red)
   0x3A8 is about 10ms (0.06% error)
   Higher PWMValue: Higher brightness
 */
    IFS0bits.T1IF = false;  // clear interrupt flag
    TMR1 = 0x0000;
    if(LedState){                   // Led is currently on
        LED_Heartbeat_SetLow();          // switch off
        PR1 = 0x3A8 - PWMValue;     // TMR1 set for off time (10ms - on time)
    }else{
        LED_Heartbeat_SetHigh();
        PR1 = PWMValue;
        if(CountUp){
            PWMValue = PWMValue + PWMStep;
        } else{
            PWMValue = PWMValue - PWMStep;
        }
        if (PWMValue > 0x100){
            PWMStep = 15;   // Value of PWMStep changes depending on duty cycle to compensate for non-linearity of led brightness
        }else{
            PWMStep = 2;
        }
        if(PWMValue>0x390){
            CountUp = 0;     // count down again
        }
        if(PWMValue<10){
            CountUp = 1;     // count up
        }
    }
    LedState = !LedState;
    /* Boot led shows that PIC has (re)started... 
     red LED_Boot starts counting down from 200, decrementing each half PWM cycle.
     if it is not 0, decrement 1
     if, after decrementing it is zero, switch off boot led
    */
    if(bootLed){                    
        --bootLed;                  // First decrement counter
        if(!bootLed){               // If zero after decrement,
            LED_Boot_SetLow();     // Switch off
        }
    }
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _T4Interrupt (  ){
/* ISR for TIMER 4 
   Generate clock to send data to mux every 50ms
*/
    IFS1bits.T4IF = false; //clear interrupt flag
    TMR4 = 0x0000;
    if(RadBuf.FillLength >0){
        RadBuf.DoMux = true;
    }
    if(IrrBuf.FillLength >0){
        IrrBuf.DoMux = true;
    }
}

void muxRad(void){
        /* Make working copies of the fill length and read position, 
     They can change if new data comes in during transmit. */
    unsigned int UxFillLengthCopy = 0;      // will hold length of message in buffer
    unsigned int UxRead = 0;                // read position in mux
    
    UxFillLengthCopy = RadBuf.FillLength;
    UxRead = RadBuf.ReadPos;
    RadBuf.ReadPos = RadBuf.WritePos;
    RadBuf.FillLength = 0;
    Uart_SendString(3, "_(1");              // Send prefix
    Uart_SendChar(3, UxFillLengthCopy);      // Send number of chars as one byte
    Uart_SendString(3,")_");
    while(UxFillLengthCopy > 0){
        Uart_SendChar(3, RadBuf.Buff[UxRead]);
        UxRead++;
        if (UxRead == BUFFLENGTH){
            UxRead = 0;
        }
        UxFillLengthCopy--;
    }
    Uart_SendChar(3, 13);            // Send CR
}

void muxIrr(void){
    /* Make working copies of the fill length and read position, 
    They can change if new data comes in during transmit. */
    unsigned int UxFillLengthCopy = 0;      // will hold length of message in buffer
    unsigned int UxRead = 0;                // read position in mux
    
    UxFillLengthCopy = IrrBuf.FillLength;
    UxRead = IrrBuf.ReadPos;
    IrrBuf.FillLength = 0;
    IrrBuf.ReadPos = IrrBuf.WritePos;
    Uart_SendString(3, "_(2");               // Send prefix
    Uart_SendChar(3, UxFillLengthCopy);      // Send number of chars as one byte
    Uart_SendString(3,")_");
    while(UxFillLengthCopy > 0){
        Uart_SendChar(3, IrrBuf.Buff[UxRead]);
        UxRead++;
        if (UxRead == BUFFLENGTH){
            UxRead = 0;
        }
        UxFillLengthCopy--;
    }
    Uart_SendChar(3, 13);            // Send CR
}

void sendVersion(void){
    Uart_SendString(4, "FW Version: ");
    Uart_SendStringNL(4, FW_VERSION);       // end with newline
}

uint8_t getVitals(void){
    // flagVitalsRequested can be 1 (requested locally) or 2(requested remotely over mux)
    // get local temp/rh, send them out of the mux (if requested remotely) or 
    // uart4 (if requested locally)
    // if requested locally, send request over mux and set flag to wait for it
    char PrintoutTemp[13] = {};
    char PrintoutHum[6] = {};
//    uint16_t i = 0;
    
    if (VitalsRequested == 1){          // vitals requested from aux serial port (local)
        // first task should be to ask other side for vitals
        flagWaitingForRemoteVitals = 1;
    }

    // get the local values
    SHT31_SingleShot(&SHT31_Temp, &SHT31_RH, 3);
    // create message, regardless of where it should be sent to
    formatVitals(PrintoutTemp, PrintoutHum);
    
    if (VitalsRequested == 1){      // local request, wait for remote response
        Uart_SendStringNL(4, PrintoutTemp);
    }
    if (VitalsRequested == 2){
        // send data to mux
    }
    return 1;
}

void formatVitals(char PrintoutTemp[], char PrintoutHum[]){
    if (ImBottom){
        strcpy(PrintoutTemp, "tb");        
        strcpy(PrintoutHum, ",hb");
    }else{                              // I'm top!
        strcpy(PrintoutTemp, "tt");        
        strcpy(PrintoutHum, ",ht");
    }
    itoa(&PrintoutTemp[2], SHT31_Temp, 10);
    itoa(&PrintoutHum[3], SHT31_RH, 10);
    strcat(PrintoutTemp, PrintoutHum);
}

void outputMuxedMsg(uint8_t TargetPort, uint16_t MsgLength, uint16_t MsgStartPos){
    while (MsgLength-- > 0){
        Uart_SendChar(TargetPort, MuxRxBuff.CircBuf[MsgStartPos++]);
        if (MsgStartPos == BUFFLENGTH){
            MsgStartPos = 0;
        }
    }
}


void processMuxedMSG(uint16_t MsgLength, uint16_t MsgStartPos){
    char ProcCommand[COMMANDMAXLENGTH] = {0};
    uint8_t ProcCommandPos = 0;
    while (MsgLength-- > 0){
        ProcCommand[ProcCommandPos++] = MuxRxBuff.CircBuf[MsgStartPos++];
        if (MsgStartPos == BUFFLENGTH){
                MsgStartPos = 0;
        }
    }
    
    if ((ProcCommand[0] == 'r')&&(ProcCommand[sizeof(ProcCommand)-1] == '*')){
        // this is a reply
        if(flagWaitingForRemoteVitals){
            
        }
    }
    
    if(strcmp (ProcCommand, "?vitals*") == 0){
        VitalsRequested = 2;
    }
}

int main(void) {
    
    uint16_t DeMuxStart = 0;
    uint16_t MuxMsgLength = 0;
    
    /*Initialize*/
    LED_Boot_SetHigh();     // After startup, light red led for 1 second (100 PWM cycles)
    initHardware();         // Init all the hardware components and setup pins
    StartWDT();
        
    
    /*Main loop*/
    while(1){
        ClrWdt();               // kick wdt

        // check if request have been received over mux or aux serial
        if (VitalsRequested > 0){
            getVitals();
            VitalsRequested = 0;
        }
        // check if version information has been requested
        if (flagVersionRequested){
            sendVersion();
            flagVersionRequested = 0;
        }
        
        if(RadBuf.DoMux){             // Send data from Radiance over mux
            muxRad();
            RadBuf.DoMux = false;    // The flag might be a new one but we don't care
        }
        
        if(IrrBuf.DoMux){               // Send data from Irr over mux
            muxIrr();
            IrrBuf.DoMux = false;           // The flag might be a new one but we don't care
        }
        
        if(flagMuxDoDemux){                 // a message has come in
            uint8_t CurrentPacketTargetPort;
            IPC20bits.U3RXIP = 0;           // disable UART3 RX interrupt
            flagMuxDoDemux = 0;             // must be before the loop to avoid race condition:
            /* If a complete new frame comes in during the while loop below,
             * and then MuxDoDemux gets set to 0 at the end,
               if no new tmr4 interrupt gets called, that buffer part never gets written out */
            MuxMsgLength = DeMuxBuff.MuxMsgLength;
            DeMuxBuff.MuxMsgLength = 0;
            DeMuxStart = MuxRxBuff.MessStart;
            CurrentPacketTargetPort = MuxRxBuff.SourcePort;
            IPC20bits.U3RXIP = 1;           //    Re-enable UART3 RX interrupt
            
            // now handle the package
            if (CurrentPacketTargetPort == 0){
                processMuxedMSG(MuxMsgLength, DeMuxStart);
            }else{
                
            }
        }
        __delay_ms(1000);
    }
    return 0;
}
