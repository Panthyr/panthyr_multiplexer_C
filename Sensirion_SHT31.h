
#ifndef XC_SHT31
#define	XC_SHT31

#include <xc.h> // include processor files - each processor file is guarded.  
#include "I2C1.h"
#include "uart.h"

//--------------------Variables--------------------
extern char AsciiResult[5];
//-------------------Macros-------------------

//--------------------Functions--------------------

int SHT31_InitReset (void);
//int SHT31_SingleShot(char*);
int SHT31_SingleShot(signed int *);
int SHT31_ConvertData(unsigned char *, signed int *);
#endif

