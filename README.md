# Panthyr multiplexer board

Firmware for the [Microchip PIC24FJ128GB204](https://www.microchip.com/wwwproducts/en/PIC24FJ128GB204) microcontroller on the (de)multiplexer board.
Compiled using the [Microchip XC16 compiler](https://www.microchip.com/mplab/compilers).

## 1. Firmware

### High level overview

The main loop first sets up the hardware (oscillator, pps, interrupts, UART and timers). It then goes into a loop that first kicks the watchdog timer, then checks for flags which are set in the interrupts and responds as appropriate.
Most work is done in the UART RX interrupt `_UxRXInterrupt` functions, as they wait for incoming data and set up the flags to notify the main loop.

### Watchdog Timer

* Started just before the main loop, after HW init
* Prescaler (`FWPSA` , config word 1 bit 4) and postscaler (`WDTPS`, cw1 bit 3-0) are both set to 128, resulting in about 512ms timeout
* After HW initialization, `RCONbits.WDTO` is checked. If set, all UARTS send out “---Reset by WDT---\n”

### Timer 1

* `T1CON = 0x020` and `PR1 = 0x3A8`: about 10ms (0.06% error)
* Used solely for heartbeat **LED_Heartbeat** (orange) and boot led **LED_Boot** (red)
* **LED_Boot** is lit after boot and remains on for 100PWM cycles (~1s). Lit again in case of errors.
* **LED_Heartbeat** is software PWM driven. Might be migrated to HW driven solution.

### Timer 4

* `T4CON = 0x2020` and `PR4 = 0x493E`: about 50ms
* Checks if there is data in `RadBuf` or `IrrBuf` and set flag for main loop to mux this data
* While these flags could also be set in the respective `_UxRXInterrupt` handlers, using the timer has the benefit of "packaging" the buffers in 50ms bursts instead of each time the main loop has run. More characters per muxed message lowers the overhead imposed by the preamble. Higher `PR4` values further lower the overhead (since this results in larger packets), but adds additional delay in the signal chain.

## 2. Ports

### UART1

* 9600 baud
* Connected/transparently mutiplexed to the radiance sensor
* Does not print anything after initialization as this can cause the instruments to malfunction
* TODO: if bottom: print init message, add top/bottom

### UART2

* 9600 baud
* Connected/transparently mutiplexed to the irradiance sensor
* Does not print anything after initialization as this can cause the instruments to malfunction
* TODO: if bottom: print init message, add top/bottom

### UART3

* 57600 baud
* MUX port (connected to the other board)
* Data format between top/bottom: `_(xy)_zzzzzzzzzzzzzzzzz<CR>`
* X is the target port: 0 for the remote cpu, 1 for radiance, 2 for irradiance, 4 for the aux port (no use yet)
* Y is the number of bytes in the payload z
* Message ends with a CR (CR is only used to end muxed messages)
* Each packet (without the preamble \_(xy)_) is checked for expected length when the CR is received. If length does not match, the packet is discarded.
* `FlagMuxDoDemux` is then set for the main loop to process the received data.
* TODO: implement CRC as more rugged check
* TODO: request re-send if failed CRC/byte count
* Prints “---Init UART3 (MUX) completed---\n" after initialization
* TODO: add top/bottom in init message

### UART4

* 57600 baud
* communicate with the uC and request data from sensors
* Prints “---Init UART4 (AUX) completed---\n" after initialization
* TODO: add top/bottom in init message

## 3. Handling received muxed packets

When the main loop sees `FlagMuxDoDemux` set it checks the target port:

* Packets for UART1 (radiance) or UART2 are then transmitted out of the respective port.
* Packets for port 0 are to be handled by the controller internally. These are handled by `processMuxedCmd`. See "Handling of incoming commands (from MUX/UART3)"

## 4. Communication with/between the microcontrollers

### Message formats

Requests/commands have the following format:

* `?xxxxxxx*` for requests
* `!xxxxxxx*` for commands
* `rxxxxxxx*` for replies to requests (or cmd ack) -> only used over the muxed port, not UART4/AUX!
No CR/LF is required (and is ignored if sent).

The two microcontrollers can communicate with each other. To do so, they send a message that is embedded in the usual form _(0y)_xxxx\CR over the multiplex (UART3) port at 57600 baud. Target port to be used is 0.

### Currently supported commands

Commands do not have to end in \n or \r (both are ignored).
These can come in through the AUX (uart4) port, or from the MUX (uart3) if originating from the remote station.
Flags can be set to 1 (command received from local AUX port) or 2 (received from the remote station).

|Command   |Returns|Explanation|Flag|
|:--------:|-------|-----------|----|
|`?vitals*`|tt2320th58\nbt2234bh24\n|local and remote temp/RH: (position:t for top, b for bottom)(t for temp)(temperature\*100)(position:t for top, b for bottom)(h for relative humidity)(relative humidity in %). Example means 23,20 deg and 58%RH for top, 22,34/24 for bottom.|`FlagVitalsRequested`|
|`?ver*`|FW Version: v0.4\n|Local firmware version|`FlagVersionRequested`|
|`?imu*`| p:(-)xxx.yy\n, r:(-)xxx.yy\n, h:xxx/n(remark1)|converted pitch/roll/heading (from top) in degrees (remark2)|`FlagImuRequested`|
|`!calibimu*`|"OK"|Re-inits IMU with calibration, referencing p/r. IMU should be level when command is executed.(remark3)|`FlagImuCalib`|

(remark1) leading zeros are not printed
(remark2) after receiving `?imu*` on aux, the local station checks if it is set to top. If so, it sends the local p/r/h. If not top, it requests the remote station for the data.
After receiving `?imu*` from the mux, it knows the remote station was not configured as top, so then checks if itself is set to top. If it is top, it returns the data, otherwise it returns --- as values.
(remark3) As only the top board uses an IMU to measure pitch/roll, this can only be performed on the top unit. In fw v0.4 it is not (yet) possible to do this from the bottom unit.

* TODO: remote FW version?

### Handling of incoming commands (from AUX/UART4)

`_U4RXInterrupt` works as a simple state machine:

1. Waiting for an exclamation mark (0x21) or question mark (0x3F) as a message start identifier.
2. It then regards the following characters as part of the incoming command and stores them in `AuxRx` and waits for asterisk (0x2A) to close the command.
3. If the length of the command exeeds `COMMANDMAXLENGTH` (50 bytes in v0.4), the buffer is emptied and goes back to step one.
4. After rx of * (0x2A), the message buffer `AuxRx` is compared to know commands (see above) and the corresponding flag is set. If the command is unknown, the machine goes back to step 1.

At this point, the `main()` loop takes over and handles the request and unsets the flag afterwards.

If the command requires communication with the remote controller, the message is send over the mux with destination port 0 and handled there (see next point).

### Handling of incoming commands (from MUX/UART3)

`processMuxedCmd` handles incoming (from mux) commands:

* It checks the validity of the message format.
* If  it is a known command, it sets the corresponding flag for the main loop to handle.
* If the message is a response to a request sent by itself (starts with "r"), it checks to see if there are any "waiting for reply"flags set. If one is set, it handles the message (output on UART4/AUX in case of `?vitals*`) and clears the flag.
* A request for vitals (`?vitals*`) sets `FlagVitalsRequested = 2`. Value 2 of this variable tells the handling function that the information is request by the remote and should be sent over the mux.

### Handling of outgoing commands (over MUX/UART3 to remote)

* A function that has to send a command to the remote, sets the `FlagTxCMDMux` flag and stores the command in the `CmdToMux` variable.
* When the `main()` loop finds the `FlagTxCMDMux`set, it calls the `muxSendCommand` function to handle this.

### Short breakdown

Send a message to remote:

* `memcpy` command to `CmdToMux`
* Set flag `FlagTxCMDMux`

Receive message from remote:

* If `FlagMuxDoDemux` is set (by `U3RXInterrupt`), main loop checks target port
* If target port == 0, `ProcessMuxedCmd` is called
* `ProcessMuxedCmd` checks if it is a reply (starting with r)
* If it is a reply, checks if any `FlagWaitingForXXX` are set. If none, discards message.
* If not a reply, checks against known commands and sets corresponding flag to 2 (received from remote)

## 5. Acknowledgements

Uses the [LSM9DS1](https://github.com/sparkfun/LSM9DS1_Breakout) code from Sparkfun to interface the IMU sensor.
