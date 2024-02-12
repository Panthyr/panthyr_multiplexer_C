#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdint.h>
#include <stdbool.h>
#include "main_globals.h"   // declarations for global variables (used in interrupts and main)
#include "utils.h"
#include "hardware.h"       // macro's for pins
#include "string.h"         // memset

volatile uint8_t AuxRxPos = 0; // Write position in the buffer

/* Variables for the heartbeat LED
   PWM cycle is 936 steps, so full off - full on cycle takes
   (936/7)*0.01s = 0.85s */
bool LedState = 0; // Heartbeat led state
uint16_t PWMValue = 0x300; // Start value for PWM duty
int8_t PWMStep = 7; // Step size for PWM
bool CountUp = 1; // For the PWM of the heartbeat led
// Used to light orange led for 2.5s after boot (400 half PWM 0.01s cycles)
uint16_t bootLed = 500; 
// Decremented each half PWM duty cycle until zero

struct CircBuf RadBuf, IrrBuf ={{0, 0, 0, 0, 0}};
struct MuxRxBuff MuxRxBuff = {{0, 0, 0, 0, 0, 0, 0}};
struct DemuxBuff DeMuxBuffDescr = {0, 0, 0};

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
        // Store data until all expected chars received
        if (MuxRxBuff.ExpectedChr > 0) { 
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
                MuxRxBuff.Preamble = 0;
                // set variables to describe the message to be demuxed
                DeMuxBuffDescr.TargetPort = MuxRxBuff.TargetPort;
                DeMuxBuffDescr.MsgLength = MuxRxBuff.MsgLength;
                DeMuxBuffDescr.MsgStartPos = MuxRxBuff.MsgStart;
                FlagMuxDoDemux = 1;
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
    // buffer is COMMANDMAXLENGTH bytes, 
    // max command is COMMANDMAXLENGTH - 2 (COMMANDMAXLENGTH - start ? or ! - end *)
    // if a known command is received, it sets its flag for the main loop to handle
    // if it gets more than COMMANDMAXLENGTH bytes, 
    // an unknown command or a new ?/! character,
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
        // start new receive operation, clear previous content
        // memset doesn't expect to be handed volatile variables,
        // So casting to (void*) as an ugly solution.
        // Since we're in the interrupt that might change te value unexpectedly,
        // I guess that's ok...
        memset((void*)AuxRx, 0, sizeof (AuxRx)); 
        AuxRx[0] = '!';
        AuxRxPos = 1; // wait for the next character to come in
        break;
    case '?': // beginning of new request
        // start new receive operation, clear previous content
        // memset doesn't expect to be handed volatile variables,
        // So casting to (void*) as an ugly solution.
        // Since we're in the interrupt that might change the value unexpectedly,
        // I guess that's ok...
        memset((void*)AuxRx, 0, sizeof (AuxRx)); 
        AuxRx[0] = '?';
        AuxRxPos = 1; // wait for the next character to come in
        break;
    case '*': // end of command/request, check what has been requested
        AuxRx[AuxRxPos++] = '*'; // add the * to the buffer and set to next position
        // add 0 (should not be needed since array was initialized with zeros?)
        AuxRx[AuxRxPos] = 0; 

        /* check for correct commands*/
        // strcmp does not expect to be handed volatile variables
        if (strcmp((void*)AuxRx, "?vitals*") == 0) {
            FlagVitalsRequested = 1;
        }
        // strcmp does not expect to be handed volatile variables
        if (strcmp((void*)AuxRx, "?ver*") == 0) {
            FlagVersionRequested = 1;
        }
        // strcmp does not expect to be handed volatile variables
        if (strcmp((void*)AuxRx, "?imu*") == 0) {
            FlagImuRequested = 1;
        }
        if (strcmp((void*)AuxRx, "!imucalib*") == 0) {
            FlagImuCalib = 1;
        }
        
        // either a valid command was passed and the appropriate flag set,
        AuxRxPos = 0; 
        // or the command was invalid. Either way, restart receive
        // memset doesn't expect to be handed volatile variables,        
        memset((void*)AuxRx, 0, sizeof (AuxRx)); 
        break;
    default: // not one of the special characters we're looking for
        if (AuxRxPos > 0) { // should already have received ? or !
            AuxRx[AuxRxPos++] = x; // add character and set to next position
        }
        break;
    }

    if (AuxRxPos > COMMANDMAXLENGTH) { 
        // too many chars to be valid, restart rx operation
        AuxRxPos = 0;
        // memset doesn't expect to be handed volatile variables,        
        memset((void*)AuxRx, 0, sizeof (AuxRx));
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
            // Value of PWMStep changes depending on duty cycle 
            // to compensate for non-linearity of led brightness
            PWMStep = 15; 
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
       Generates clock to send data to mux every 50ms
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