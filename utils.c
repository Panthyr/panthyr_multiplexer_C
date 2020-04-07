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
    // add sign to end (will be beginning) of string
    if (neg){
        str[pos++]='-';
    }
  
    // If number of digits required is more, then 
    // add 0s at the beginning (now end)
    while (pos < minChar) 
        str[pos++] = '0'; 
  
    reverse(str, pos); 
    str[pos] = '\0'; 
    return pos; 
} 
  
// Converts a floating-point/double number to a string.
// The high level function
uint16_t ftoa(float floatToConvert, char* buf, uint8_t afterpoint) 
{   
    uint8_t neg = 0;
    
    // check if negative number (which wasn't handled to well if between 0 and -1)
    if (floatToConvert < 0){
        neg = 1;
        floatToConvert = -floatToConvert;
    }
    // Extract integer part 
    int integerPart = (int)floatToConvert; 
      // Extract floating part 
    float floatRemainder = floatToConvert - (float)integerPart; 
      // convert integer part to string 
    uint16_t charsUsed = intToStr(integerPart, buf, 1, neg); 
      // check for display option after point 
    if (afterpoint != 0) { 
        buf[charsUsed] = '.'; // add dot 
  
        // Get the value of fraction part upto given no. 
        // of points after dot. The third parameter  
        // is needed to handle cases like 233.007 
        floatRemainder = floatRemainder * pow(10, afterpoint);
        charsUsed += intToStr((int)floatRemainder, buf + charsUsed + 1, afterpoint, 0);
    } 
    return charsUsed;
} 
