/*
*******************************************************************************
*                                  AVR-FAT MODULE
*
* File    : PRINTS.C
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
* License : MIT
* Copyright (c) 2020
* 
*
* DESCRIPTION:
* Declare some print functions used to print strings and positive integers in 
* decimal, binary, and hex formats.
*******************************************************************************
*/


#include <stdint.h>
#include <avr/io.h>
#include "../includes/prints.h"
#include "../includes/usart.h"





/*
*******************************************************************************
*******************************************************************************
 *                     
 *                                 FUNCTIONS   
 *  
*******************************************************************************
*******************************************************************************
*/

/*
-------------------------------------------------------------------------------
|                  PRINT POSITIVE DECIMAL (BASE-10) INTEGERS 
|                                        
| Description : Prints unsigned base-10 integers to a screen.
|
| Argument    : num    - Unsigned integer to be printed to the screen.
-------------------------------------------------------------------------------
*/

void 
print_dec (uint32_t num)
{
    //Determine number of decimal digits required to represent the number.
    //32 bit integer can have up to 10 digits
    uint8_t len = 1;

    if(num < 10)                len = 1;
    else if(num <  100)         len = 2;
    else if(num <  1000)        len = 3;
    else if(num <  10000)       len = 4;
    else if(num <  100000)      len = 5;
    else if(num <  1000000)     len = 6;
    else if(num <  10000000)    len = 7;
    else if(num <  100000000)   len = 8;
    else if(num <  1000000000)  len = 9;
    else if(num <= 4294967295)  len = 10; //largest 32-bit integer value

    char dec[len];

    // initialize each digit to 0.
    for (uint8_t i = 0; i < len; i++) 
      dec[i] = '0'; 

    uint32_t result = num;
    uint8_t  remainder = 0;

    for (uint8_t i = 0; i < len; i++)
    {
        remainder = result % 10;
        result = result / 10;

        // convert digit to ascii character
        dec[i] = remainder + 48;
    }
    
    //print characters in decimal array
    for (int i = len-1; i >= 0; i--) 
      usart_transmit (dec[i]); 
}
//END print_int()



/*
-------------------------------------------------------------------------------
|                           PRINT BINARY FORM OF INTEGER 
|                                        
| Description : Prints the binary form of the unsigned integer argument.
|
| Argument    : num    - Unsigned integer to be printed to the screen.
|
| Notes       : Prints the number as space-separated nibbles (4bits), and only 
|               the number of required nibbles will be printed.
-------------------------------------------------------------------------------
*/

void 
print_bin (uint32_t num)
{
    //Determine number binary digits required to represent the number.
    uint8_t len = 4;
    
    if(num < 0x10)              len = 4;
    else if(num < 0x100)        len = 8;
    else if(num < 0x1000)       len = 12;
    else if(num < 0x10000)      len = 16;
    else if(num < 0x100000)     len = 20;
    else if(num < 0x1000000)    len = 24;
    else if(num < 0x10000000)   len = 28;
    else if(num <= 0xFFFFFFFF)  len = 32;

    char bin[len];

    //initialize each digit to '0'.
    for (uint8_t i = 0; i < len; i++) 
      bin[i] = '0'; 
    
    uint32_t result = num;
    uint8_t  remainder = 0;

    for (uint8_t i = 0; i < len; i++)
    {
        remainder = result % 2;
        result = result / 2;

        if (remainder == 0) 
            bin[i] = '0';
        else 
            bin[i] = '1';
    }

    // print characters in binary array. 
    // print a space every 4 bits.
    for (int i = len-1; i >= 0; i--)
    {
        usart_transmit (bin[i]);
        if ((i % 4) == 0) 
          usart_transmit (' ');
    }
}
//END print_bin()



/*
-------------------------------------------------------------------------------
|                      PRINT HEXADECIMAL FORM OF AN INTEGER
|                                        
| Description : Prints the hexadecimal form of the unsigned integer argument.
|
| Argument    : num    - Unsigned integer to be printed to the screen.
-------------------------------------------------------------------------------
*/

void 
print_hex(uint32_t num)
{
    //Determine number of hexadecimal digits required to represent the number.
    uint8_t len = 2;
    
    if (num < 0x100)           len = 2;
    else if(num < 0x1000)      len = 3;
    else if(num < 0x10000)     len = 4;
    else if(num < 0x100000)    len = 5;
    else if(num < 0x1000000)   len = 6;
    else if(num < 0x10000000)  len = 7;
    else if(num <= 0xFFFFFFFF) len = 8;

    char hex[len];
    
    // initialize each digit to '0'.
    for (int i = 0; i < len; i++) 
      hex[i] = '0';
    
    uint32_t result = num;
    uint8_t  remainder = 0;

    for (int i = 0; i < len; i++)
    {
        remainder = result % 16;
        result = result / 16;

        //convert to ascii characters.
        if ((remainder < 10) && (remainder >= 0))
          hex[i] = remainder + 48; //ascii numbers
        else if ((remainder >= 10) && (remainder <= 16)) 
          hex[i] = (remainder - 10) + 65; //ascii letters
    }
    
    //print characters in hexadecimal array.
    for (int i = len-1; i >= 0; i--) 
      usart_transmit (hex[i]);
}    
//END print_hex()



/*
-------------------------------------------------------------------------------
|                                 PRINT C-STRING
|                                        
| Description : Prints the C-string passed as the argument.
|
| Argument    : str  - ptr to the string that will be printed to the terminal. 
|
| Notes:      : (1)  C-strings are null-terminated '\0'.
|               (2)  Only strings of 999 characters + NULL are currently
|                    handled by this function.
-------------------------------------------------------------------------------
*/

void 
print_str(char * str)
{
    int i = 0;
    while(str[i] != '\0' && i < 1001)
    {
        usart_transmit(str[i]);
        i++;
    };
}
//END print_str()
