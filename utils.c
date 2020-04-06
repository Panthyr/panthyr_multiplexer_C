#include <math.h>
#include "uart.h"
#include "utils.h"
  

void __ISR _DefaultInterrupt(void)
{
    Uart_SendStringNL(1, "DefaultInterrupt");
    Uart_SendStringNL(2, "DefaultInterrupt");
    Uart_SendStringNL(3, "DefaultInterrupt");
    Uart_SendStringNL(4, "DefaultInterrupt");
    while(1);  // hold here so we can read UART messages
}

void __ISR _MathError(void)
{
    Uart_SendStringNL(1, "_MATHERR");
    Uart_SendStringNL(2, "_MATHERR");
    Uart_SendStringNL(3, "_MATHERR");
    Uart_SendStringNL(4, "_MATHERR");
    while(1);  // hold here so we can read UART messages
}

// Reverses a string 'str' of length 'len'
// for use with ftoa
void reverse(char* str, int len) 
{ 
    int i = 0, j = len - 1, temp; 
    while (i < j) { 
        temp = str[i]; 
        str[i] = str[j]; 
        str[j] = temp; 
        i++; 
        j--; 
    } 
} 
  
// Converts a given integer x to string str[].  
// d is the number of digits required in the output.  
// If d is more than the number of digits in x,  
// then 0s are added at the beginning. 
// for use with ftoa
int intToStr(uint16_t value, char str[], int minChar, uint8_t neg) 
{ 
    int pos = 0;  // start at beginning
    
    if (value == 0){
        str[pos++] = '0';
    }
    
    while (value) { 
        str[pos++] = (value % 10) + 0x30; // '0' character used as offset to convert from int to ANSII
        value = value / 10; 
    };
    if (neg){
        str[pos++]='-';
    }
  
    // If number of digits required is more, then 
    // add 0s at the beginning 
    while (pos < minChar) 
        str[pos++] = '0'; 
  
    reverse(str, pos); 
    str[pos] = '\0'; 
    return pos; 
} 
  
// Converts a floating-point/double number to a string.
// The high level function
void ftoa(float number, char* buf, uint8_t afterpoint) 
{   
    uint8_t neg = 0;
    
    // check if negative number (which wasn't handled to well if between 0 and -1)
    if (number < 0){
        neg = 1;
        number = -number;
    }
    // Extract integer part 
    int ipart = (int)number; 
      // Extract floating part 
    float fpart = number - (float)ipart; 
      // convert integer part to string 
    int i = intToStr(ipart, buf, 1, neg); 
      // check for display option after point 
    if (afterpoint != 0) { 
        buf[i] = '.'; // add dot 
  
        // Get the value of fraction part upto given no. 
        // of points after dot. The third parameter  
        // is needed to handle cases like 233.007 
        fpart = fpart * pow(10, afterpoint); 
        intToStr((int)fpart, buf + i + 1, afterpoint, 0); 
    } 
} 
