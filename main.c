/*
 * Hypermaq/Panthyr serial port multiplex
 * SW version v0.3
 * For HW TOP v0.2, Bottom v0.1
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


#include <xc.h>
#include "main.h"

/* Variables for the UARTS */
#define BUFFLENGTH 1024

struct CircBuf{
    char Buff[BUFFLENGTH];
    unsigned int WritePos;           // Write position
    unsigned int ReadPos;            // Read position
    unsigned int FillLength;      // Number of unprocessed chars in array
    bool DoMux;                   // Flag if there's buffered RX from Ux
}RadBuf, IrrBuf = {0,0,0,0,0};

char MuxCircBuf[BUFFLENGTH] = {0};   // Circular buffer for UART3
unsigned int MuxWrite = 0;           // Write position
unsigned int MuxMessStart = 0;            // Read position
unsigned int MuxFillLength = 0;      // Number of unprocessed chars in array
unsigned int MuxSourcePort = 0;     // Where did the message from?
unsigned int MuxExpectedChr = 0;    // Counter for number of expected chars
unsigned int MuxPreamble = 0;       // Counter for preamble
unsigned char ToSendData[BUFFLENGTH] = {}; // Message that needs to be send out
unsigned int ToSendPort = 1;         // Where this message should go
unsigned int MuxReceived = 0;       // Number of received chars in message
bool MuxDoDemux = 0;                 // Flag if there's buffered RX from U3

/* Variables for the heartbeat LED
   PWM cycle is 936 steps, so full off - full on cycle takes
   (936/7)*0.01s = 0.85s */
bool LedState = 0;              // Heartbeat led state
unsigned int PWMValue = 0x300;  // Start value for PWM duty
signed int PWMStep = 7;         // Step size for PWM
bool CountUp = 1;               // For the PWM of the heartbeat led
unsigned int bootLed = 500;     // Used to light orange led for 2.5s after boot (400 half PWM 0.01s cycles)
                                // Decremented each half PWM duty cycle until zero

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U1TXInterrupt ( void ){ 
    IFS0bits.U1TXIF = false;
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U1RXInterrupt ( void ){ 
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

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U2TXInterrupt ( void ){ 
    IFS1bits.U2TXIF = false;
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U2RXInterrupt ( void ){ 
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

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U3TXInterrupt ( void ){ 
    IFS5bits.U3TXIF = false;
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U3RXInterrupt ( void ){ 
    IFS5bits.U3RXIF = false; // clear interrupt bit
    LED_Mux_Rx_Toggle();
    unsigned char x = U3RXREG;
    switch(MuxPreamble){
        case 0:                             // Waiting for start of muxed message
            if (x == '_'){                  // Correct start char
//                while(blockU3RxVars){};
                MuxMessStart = MuxWrite;    // "reserve" start position
                MuxPreamble++;              // One char of preamble correct
            } 
            break;
        case 1:
            if (x == '('){
                MuxPreamble++;
            } else {MuxPreamble =0;         // Incorrect preamble, restart
                LED_Boot_Toggle();
            }
            break;
        case 2:
            if (x - 0x30 < 5){                     // Port number should be 1,2 or 4
                MuxSourcePort = x - 0x30;
                MuxPreamble++;
            } else {
                LED_Boot_Toggle();
                MuxPreamble = 0;
            }
            break;
        case 3:                             // _(x was already received
            if (x > 0){
                MuxExpectedChr = x;
                MuxPreamble++;
            } else { 
                LED_Boot_Toggle();
                MuxPreamble = 0;
            }
            break;
        case 4:
            if (x == ')'){
                MuxPreamble++;
            } else {
                LED_Boot_Toggle();
                MuxPreamble =0;         // Incorrect preamble, restart
            }
            break;
        case 5:
            if (x == '_'){
                MuxPreamble++;
                break;
            } else {
                MuxPreamble =0;         // Incorrect preamble, restart
                LED_Boot_Toggle();
            }
            break;
        case 6:
            if (MuxExpectedChr > 0){           // Store data until all expected chars received
                MuxCircBuf[MuxWrite] = x;
                MuxExpectedChr--;
//                while(blockU3RxVars){};
                MuxReceived++;
                MuxWrite++;
                if (MuxWrite == BUFFLENGTH){
                    MuxWrite = 0;
                }
                break;
             } else if ( x == 13){
                //send data out
                MuxDoDemux = 1;
                MuxPreamble = 0;
             } else {                           // No CR when expected, dump message
                MuxPreamble =0;
                LED_Boot_Toggle();
             }
    }
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U4TXInterrupt ( void ){ 
    IFS5bits.U4TXIF = false;
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U4RXInterrupt ( void ){ 
    IFS5bits.U4RXIF = false; // clear interrupt bit
    LED_Sens_Rx_Toggle();
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
//    TP32_Toggle();  // Toggle test point as test
    if(RadBuf.FillLength >0){
        RadBuf.DoMux = true;
    }
    if(IrrBuf.FillLength >0){
        IrrBuf.DoMux = true;
    }
//    if(MuxReceived >0){
//        MuxDoDemux = true;
//    }
}

int main(void) {
    
    /*Initialize*/
    initHardware();         // Init all the hardware components
    LED_Boot_SetHigh();         // After startup, light red led for 1 second (100 PWM cycles)
    
    StartWDT();
    
    /*Init completed*/
    unsigned int UxFillLengthCopy = 0;
    unsigned int UxRead = 0;  
    unsigned int DeMuxStart = 0;
    unsigned int DeMuxTodo = 0;
    /*Main loop*/
    while(1){
        ClrWdt();
        if(RadBuf.DoMux){
            /* Make working copies of the fill length and read position, 
             They can change if new data comes in during transmit. */
            UxFillLengthCopy = RadBuf.FillLength;
            UxRead = RadBuf.ReadPos;
            RadBuf.ReadPos = RadBuf.WritePos;
            RadBuf.FillLength = 0;
            SendString(3, "_(1");              // Send prefix
            SendChar(3, UxFillLengthCopy);      // Send number of chars as one byte
            SendString(3,")_");
            while(UxFillLengthCopy > 0){
                SendChar(3, RadBuf.Buff[UxRead]);
                UxRead++;
                if (UxRead == BUFFLENGTH){
                    UxRead = 0;
                }
                UxFillLengthCopy--;
            }
            SendChar(3, 13);            // Send CR
            RadBuf.DoMux = false;    // The flag might be a new one but we don't care
        }
        
        if(IrrBuf.DoMux){
            /* Make working copies of the fill length and read position, 
             They can change if new data comes in during transmit. */
            UxFillLengthCopy = IrrBuf.FillLength;
            UxRead = IrrBuf.ReadPos;
            IrrBuf.FillLength = 0;
            IrrBuf.ReadPos = IrrBuf.WritePos;
            SendString(3, "_(2");              // Send prefix
            SendChar(3, UxFillLengthCopy);      // Send number of chars as one byte
            SendString(3,")_");
            while(UxFillLengthCopy > 0){
                SendChar(3, IrrBuf.Buff[UxRead]);
                UxRead++;
                if (UxRead == BUFFLENGTH){
                    UxRead = 0;
                }
                UxFillLengthCopy--;
            }
            SendChar(3, 13);            // Send CR
            IrrBuf.DoMux = false;           // The flag might be a new one but we don't care
        }
        
        if(MuxDoDemux){
            IPC20bits.U3RXIP = 0;  //    Disable UART3 RX interrupt
            MuxDoDemux = 0;             // must be before the loop to avoid race
            /* If a complete new frame comes in during the while loop below,
             * and then MuxDoDemux gets set to 0 at the end,
               if no new tmr4 interrupt gets called, that buffer part never gets written out */
            DeMuxTodo = MuxReceived;
            MuxReceived = 0;
            DeMuxStart = MuxMessStart;
            IPC20bits.U3RXIP = 1;  //    Re-enable UART3 RX interrupt
            while (DeMuxTodo > 0){
                SendChar(MuxSourcePort, MuxCircBuf[DeMuxStart]);
                DeMuxStart++;
                if (DeMuxStart == BUFFLENGTH){
                    DeMuxStart = 0;
                }
                DeMuxTodo--;
            }
        }
    }
    return 0;
}
