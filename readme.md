# 1.Ports

## UART1
*	9600 baud
*	Connected/transparently mutiplexed to the radiance sensor
*	Does not print anything after initialization as this can cause the instruments to malfunction
*	TODO: if bottom: print init message, add top/bottom
## UART2
*	9600 baud
*	Connected/transparently mutiplexed to the irradiance sensor
*	Does not print anything after initialization as this can cause the instruments to malfunction
*	TODO: if bottom: print init message, add top/bottom
## UART3
*	57600 baud
*	MUX port (connected to the other board)
*	Data format between top/bottom: _(xy)_zzzzzzzzzzzzzzzzzCR
*	X is the target port: 0 for the remote cpu, 1 for radiance, 2 for irradiance, 4 for the aux port (no use yet)
*	Y is the number of bytes in the payload z
*	Message ends with a CR (CR is only used to end muxed messages)
*	Each packet (without the preamble _(xy)_) is checked for expected length when the CR is received. If length does not match, the packet is discarded.
*	TODO: implement CRC as more rugged check
*	TODO: request re-send if failed CRC/byte count
*	Prints “---Init UART3 (MUX) completed---\n" after initialization
*	TODO: add top/bottom in init message
## UART4 
*	57600 baud 
*	communicate with the uC and request data from sensors
*	Prints “---Init UART4 (AUX) completed---\n" after initialization
*	TODO: add top/bottom in init message

# 1.Communication with/between the microcontrollers

## Message formats
Requests/commands have the following format:
?xxxxxxx* for requests
!xxxxxxx* for commands
rxxxxxxx* for replies to requests (or cmd ack)
No CR/LF is required (and is ignored if sent).

The two microcontrollers can communicate with each other. To do so, they send a message that is embedded in the usual form _(0y) over the multiplex (UART3) port at 57600 baud.
 
# 1.FW 

## High level overview
The main loop first sets up the hardware (oscillator, pps, interrupts, UART and timers). It then goes into a loop that first kicks the watchdog timer, then checks for flags which are set in the interrupts and responds as appropriate.
##	WDT
*	Started just before the main loop
*	Pre- (FWPSA , config word 1 bit 4) and postscaler(WDTPS, cw1 bit 3-0) are both set to 128, resulting in about 512ms timeout
*	After HW initialization, RCONbits.WDTO is checked. If set, all UARTS send out “---Reset by WDT---\n”

## Timer 1

## Timer 4
