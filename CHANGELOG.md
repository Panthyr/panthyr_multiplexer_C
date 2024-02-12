# Changelog

## v0.4

- Added communication/protocol between local and remote
- Pragma's moved to `config.h`
- Circular buffers restructured
- Variables declared as volatile when required
- Removed Uart Tx ISR's (and disabled interrupts)

## v0.5

- Moved all interrupts to `interrupts.c` and globals to `main_globals.h`

## v0.5.1 (12/08/2020)

- version v0.2.1 of `Sensirion_SHT31.c` (adress 0x44)
