#include <xc.h>
#include "hardware.h"
#include "I2C1.h"
#include "uart.h"
#include "Sensirion_SHT31.h"
#include "ST_LSM9DS1.h"

void initHardware( void ){
    
    initOscillator();
    initPPS();
    initInterrupts();
    I2C1_Init();
    InitU1();
    InitU2();
    InitU3();
    InitU4();
    initTimer1();
    initTimer4();
    checkIfResetByWDT();
    
    /* TP */    
    TP32_SetDigOut();
    TP35_SetDigOut();
    TP19_SetDig();
    TP19_SetDigOut();
        
    /* LEDS */
    LED_Sens_Rx_SetDigOut();
    LED_Mux_Rx_SetDigOut();
    LED_Heartbeat_SetDigOut();
    LED_Boot_SetDigOut();
    
    /* SWITCH */
    TopBotSwitch_SetDigIn();

    Uart_SendStringNL(4, "HW init done.");
    Uart_SendString(4, "Board set to ");
    if(ImTop){
        Uart_SendStringNL(4, "<TOP>");
    }
    if(ImBottom){
        Uart_SendStringNL(4, "<BOTTOM>");
    }
}

void initPPS( void ){
    __builtin_write_OSCCONL(OSCCON & 0xbf); // Special command to unlock PPS
    
    
    /* PINOUT FOR TOP v0.2 / BOTTOM v0.1 */
    // Outputs are set by assigning a function number to the pin address
    // | RP | address          | function no
    // | 23 | RPOR11bits.RP23R | 3  (U1Tx)
    // | 24 | RPOR12bits.RP24R | 5  (U2Tx)
    // |  7 | RPOR3bits.RP7R   | 19 (U3Tx)
    // | 19 | RPOR9bits.RP19R  | 21 (U4Tx)
    
    // Inputs are set by setting the pin number to the input function address
    // | Func | address           | RP
    // | U1Rx | RPINR18bits.U1RXR | 22 (0x16)
    // | U2Rx | RPINR19bits.U2RXR | 13 (0xD)
    // | U3Rx | RPINR17bits.U3RXR | 25 (0x19)
    // | U4Rx | RPINR27bits.U4RXR | 20 (0x14)
    
    //UART1    
    RPOR11bits.RP23R = 0x0003;    // (U1Tx) RP23 pin gets function 3 (U1Tx)
    RPINR18bits.U1RXR = 0x0016;    //(U1Rx) Function U1Rx to pin 0x26 (rp22)

    //UART2    
    RPOR12bits.RP24R = 0x0005;    // (U2Tx) RP24 pin gets function 5 (U2Tx)
    RPINR19bits.U2RXR = 0x000D;    //(U2Rx) Function U2Rx to pin 0x0D (RP13)

    //UART3
    RPOR3bits.RP7R = 0x0013;    // (U3Tx) RP7 pin gets function 19 (U3Tx)
    RPINR17bits.U3RXR = 0x0019;    //(U3Rx) Function U3Rx to pin 0x19 (RP25)

    //UART4
    RPOR9bits.RP19R = 0x0015;    // (U4Tx) RP19 pin gets function 21 (U4Tx)
    RPINR27bits.U4RXR = 0x0014;    //(U3Rx) Function U3Rx to pin 0x14 (RP20)
    
    __builtin_write_OSCCONL(OSCCON | 0x40); // lock   PPS
}
void initOscillator( void ){
    // CF no clock failure; NOSC FRCPLL; SOSCEN enabled; POSCEN disabled; CLKLOCK unlocked; OSWEN Switch is Complete; IOLOCK not-active; 
    __builtin_write_OSCCONL((uint8_t) (0x0102 & 0x00FF));
    // CPDIV 1:1; PLLEN enabled; RCDIV FRC/4; DOZE 1:8; DOZEN disabled; ROI disabled; 
    CLKDIV = 0x3220;
    // STOR disabled; STORPOL Interrupt when STOR is 1; STSIDL disabled; STLPOL Interrupt when STLOCK is 1; STLOCK disabled; STSRC SOSC; STEN enabled; TUN Center frequency; 
    OSCTUN = 0x8000;
    // ROEN disabled; ROSEL FOSC; ROSIDL disabled; ROSWEN disabled; ROOUT disabled; ROSLP disabled; 
    REFOCONL = 0x0000;
    // RODIV 0; 
    REFOCONH = 0x0000;

    /* starting at compiler XC16v 1.36, REFOTRIML should be replaced by REFOTRIMH as this was an error in older h files.
     * the REFOTRIML did not really exist, though it is mentioned in the (non-updated) datasheets, MCC generators and h files
     * (see: https://www.microchip.com/forums/m1130622.aspx)
     */
//    // ROTRIM 0; 
//    REFOTRIML = 0x0000;
    // ROTRIM 0; 
    REFOTRIMH = 0x0000;
}
void initInterrupts (void) {
    /* Set priority for interrupts: uart Rx/Tx and Timer 1 and 4 */
    IPC3bits.U1TXIP = 0;    // Disable Uart1 TX interrupt
    IPC2bits.U1RXIP = 1;    // URXI: U1RX - UART1 Receiver     Priority: 1
    IPC7bits.U2TXIP = 0;    // Disable Uart2 TX interrupt
    IPC7bits.U2RXIP = 1;    // URXI: U2RX - UART2 Receiver     Priority: 1
    IPC20bits.U3TXIP = 0;   // Disable Uart3 TX interrupt
    IPC20bits.U3RXIP = 1;   // URXI: U3RX - UART3 Receiver     Priority: 1   
    IPC22bits.U4TXIP = 0;   // Disable Uart4 TX interrupt
    IPC0bits.T1IP = 1;      // TI: T1 - Timer1    Priority: 2
    IPC6bits.T4IP = 1;      // T4: T4 - Timer4    Priority: 2
}

void InitU1(void){
    
    /* UART LINK */
    U1_Tx_SetDigOut();
    U1_Tx_SetHighOut();
    U1_Rx_SetDigIn();   
    
   // STSEL 1; IREN disabled; PDSEL 8N; UARTEN disabled; RTSMD disabled; USIDL disabled; WAKE disabled; ABAUD disabled; LPBACK disabled; BRGH enabled; URXINV disabled; UEN TX_RX; 
   U1MODE = 0x0008;
   // UTXISEL0 TX_ONE_CHAR; UTXINV disabled; OERR NO_ERROR_cleared; URXISEL RX_ONE_CHAR; UTXBRK COMPLETED; UTXEN disabled; ADDEN disabled; 
   U1STA = 0x0000;
    // BaudRate = 9600; Frequency = 6000000 Hz; U1BRG 77; 
   U1BRG = 0x9B;
   // ADMADDR 0; ADMMASK 0; 
   U1ADMD = 0x0000;
   // T0PD 1 ETU; PTRCL T0; TXRPT Retransmits the error byte once; CONV Direct; SCEN disabled; 
   U1SCCON = 0x0000;
   // TXRPTIF disabled; TXRPTIE disabled; WTCIF disabled; WTCIE disabled; PARIE disabled; GTCIF disabled; GTCIE disabled; RXRPTIE disabled; RXRPTIF disabled; 
   U1SCINT = 0x0000;
   // GTC 0; 
   U1GTC = 0x0000;
   // WTCL 0; 
   U1WTCL = 0x0000;
   // WTCH 0; 
   U1WTCH = 0x0000;

   IEC0bits.U1RXIE = 1;

    //Make sure to set LAT bit corresponding to TxPin as high before UART initialization
   U1MODEbits.UARTEN = 1;  // enabling UART ON bit
   U1STAbits.UTXEN = 1;
}

void InitU2(void){
    
    /* UART LINK */
    U2_Tx_SetDigOut();
    U2_Tx_SetHighOut();
    U2_Rx_SetDig();         // Pin B13 is analog by default
    U2_Rx_SetDigIn();   
    
   // STSEL 1; IREN disabled; PDSEL 8N; UARTEN disabled; RTSMD disabled; USIDL disabled; WAKE disabled; ABAUD disabled; LPBACK disabled; BRGH enabled; URXINV disabled; UEN TX_RX; 
   U2MODE = 0x0008;  // disabling UARTEN bit   
   // UTXISEL0 TX_ONE_CHAR; UTXINV disabled; OERR NO_ERROR_cleared; URXISEL RX_ONE_CHAR; UTXBRK COMPLETED; UTXEN disabled; ADDEN disabled; 
   U2STA = 0x0000;
   // BaudRate = 9600; Frequency = 6000000 Hz; U1BRG 77; 
   U2BRG = 0x9B;
   // ADMADDR 0; ADMMASK 0; 
   U2ADMD = 0x0000;
   // T0PD 1 ETU; PTRCL T0; TXRPT Retransmits the error byte once; CONV Direct; SCEN disabled; 
   U2SCCON = 0x0000;
   // TXRPTIF disabled; TXRPTIE disabled; WTCIF disabled; WTCIE disabled; PARIE disabled; GTCIF disabled; GTCIE disabled; RXRPTIE disabled; RXRPTIF disabled; 
   U2SCINT = 0x0000;
   // GTC 0; 
   U2GTC = 0x0000;
   // WTCL 0; 
   U2WTCL = 0x0000;
   // WTCH 0; 
   U2WTCH = 0x0000;

   // Enable interrupt for UART2Rx
   IEC1bits.U2RXIE = 1;

    //Make sure to set LAT bit corresponding to TxPin as high before UART initialization
   U2MODEbits.UARTEN = 1;  // enabling UART ON bit
   U2STAbits.UTXEN = 1;
}

void InitU3(void){
   /* UART LINK */
    U3_Tx_SetDigOut();
    U3_Tx_SetHighOut();
    U3_Rx_SetDigIn();   
    
   // STSEL 1; IREN disabled; PDSEL 8N; UARTEN disabled; RTSMD disabled; USIDL disabled; WAKE disabled; ABAUD disabled; LPBACK disabled; BRGH enabled; URXINV disabled; UEN TX_RX; 
   U3MODE = 0x0008;  // disabling UARTEN bit   
   // UTXISEL0 TX_ONE_CHAR; UTXINV disabled; OERR NO_ERROR_cleared; URXISEL RX_ONE_CHAR; UTXBRK COMPLETED; UTXEN disabled; ADDEN disabled; 
   U3STA = 0x0000;
   // BaudRate = 57600; Frequency = 6000000 Hz; U1BRG 77; 
   U3BRG = 0x19;
   // ADMADDR 0; ADMMASK 0; 
   U3ADMD = 0x0000;

   // Enable interrupt for UART3Rx
   IEC5bits.U3RXIE = 1;

    //Make sure to set LAT bit corresponding to TxPin as high before UART initialization
   U3MODEbits.UARTEN = 1;  // enabling UART ON bit
   U3STAbits.UTXEN = 1;

   Uart_SendString(3, "---Init UART3 (MUX) completed---\n");
}

void InitU4(void){
   /* UART LINK */
    U4_Tx_SetDigOut();
    U4_Tx_SetHighOut();
    U4_Rx_SetDigIn();   
    
   // STSEL 1; IREN disabled; PDSEL 8N; UARTEN disabled; RTSMD disabled; USIDL disabled; WAKE disabled; ABAUD disabled; LPBACK disabled; BRGH enabled; URXINV disabled; UEN TX_RX; 
   U4MODE = 0x0008;  // disabling UARTEN bit   
   // UTXISEL0 TX_ONE_CHAR; UTXINV disabled; OERR NO_ERROR_cleared; URXISEL RX_ONE_CHAR; UTXBRK COMPLETED; UTXEN disabled; ADDEN disabled; 
   U4STA = 0x0000;
   // BaudRate = 57600; Frequency = 6000000 Hz;
   U4BRG = 0x19;
   // ADMADDR 0; ADMMASK 0; 
   U4ADMD = 0x0000;

   // Enable interrupt for UART4Rx
   IEC5bits.U4RXIE = 1;

    //Make sure to set LAT bit corresponding to TxPin as high before UART initialization
   U4MODEbits.UARTEN = 1;  // enabling UART ON bit
   U4STAbits.UTXEN = 1;
   Uart_SendString(4, "---Init UART4 (AUX) completed---\n");
}

void checkIfResetByWDT( void ){
    if (RCONbits.WDTO){
       Uart_SendString(1, "---Reset by WDT---\n");
       Uart_SendString(2, "---Reset by WDT---\n");
       Uart_SendString(3, "---Reset by WDT---\n");
       Uart_SendString(4, "---Reset by WDT---\n");
   }
}

void initTimer1( void ){
         
    //TMR1 0; 
    TMR1 = 0x0000;
    //TCKPS 1:64; TON disabled; TSIDL disabled; TCS FOSC/2; TECS SOSC; TSYNC disabled; TGATE disabled; 
    T1CON = 0x020;
    // Period: 10ms
    PR1 = 0x3A8;
    
    IEC0bits.T1IE = true;    // Enable the interrupt
    T1CONbits.TON = 1;       // Start Timer
}

void initTimer4(void){
    // TMR4 disabled, TSIDL Stop in idle, TECS SOSC, TGATE disable, TCKPS /64, T32 16 bit, TCS Fosc
    T4CON = 0x2020;
    //Set to zero
    TMR4 = 0x0000;
    // period of 200ms
    PR4 = 0x493E;
    // enable interrupt
    IEC1bits.T4IE = 1;
    
    
    // start timer
    T4CONbits.TON = 1;
    
}