/*
***********************************************************************************************************************
*                                                   AVR-GENERAL MODULE
*
* File    : PRINTS.C
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
*
*
* DESCRIPTION:
* Defines some print functions from PRINTS.H to print strings and positive integers in decimal, binary, hex formats.
*
*
* FUNCTIONS:
*   (1) void print_dec (uint32_t num);
*   (2) void print_bin (uint32_t num);
*   (3) void print_hex (uint32_t num);
*   (4) void print_str (char * str);
*
*
*                                                       MIT LICENSE
*
* Copyright (c) 2020 Joshua Fain
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
* documentation files (the "Software"), to deal in the Software without restriction, including without limitation the 
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to 
* permit ersons to whom the Software is furnished to do so, subject to the following conditions: The above copyright 
* notice and this permission notice shall be included in all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***********************************************************************************************************************
*/


#include <stdint.h>
#include <avr/io.h>
#include "../includes/prints.h"
#include "../includes/usart.h"


/*
***********************************************************************************************************************
 *                                        PRINT POSITIVE DECIMAL (BASE-10) INTEGERS
 * 
 * Description : Prints unsigned (positive) base-10 (decimal) integers to a terminal.
 *  
 * Argument    : n   Unsigned integer to be printed to the terminal/screen in decimal form.
***********************************************************************************************************************
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
      USART_Transmit (dec[i]); 
}
//END print_int(...)



/*
***********************************************************************************************************************
 *                                            PRINT BINARY FORM OF INTEGER
 * 
 * Description : Prints the binary form of the unsigned integer argument.
 *  
 * Argument    : n   Unsigned integer to be printed to the terminal/screen in binary form.
 * 
 * Notes       : This function prints the number as space-separated nibbles (4bits), and only the number of required
 *               nibbles will be printed. required.
***********************************************************************************************************************
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
        USART_Transmit (bin[i]);
        if ((i % 4) == 0) 
          USART_Transmit (' ');
    }
}
//END print_bin(...)



/*
***********************************************************************************************************************
 *                                        PRINT HEXADECIMAL FORM OF AN INTEGER
 * 
 * Description : Prints the hexadecimal form of the unsigned integer argument.
 *  
 * Argument    : n   Unsigned integer to be printed to the terminal/screen in hexadecimal form.
***********************************************************************************************************************
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
      USART_Transmit (hex[i]);
}    
//END print_hex(...)



/*
***********************************************************************************************************************
 *                                                  PRINT C-STRING
 * 
 * Description : Prints the C-string passed as the argument.
 *  
 * Argument    : n   Pointer to C-String char array that will be printed to the terminal. 
 * 
 * Notes:      : (1)  C-strings are null-terminated '\0', and therefore this function will only work if a char array
 *                    contains a NULL. If it doesn't then unexpected results will likely occur.
 *               (2)  Only strings of 999 characters + NULL are currently handled by this function.
***********************************************************************************************************************
*/
void 
print_str(char * str)
{
    int i = 0;
    while(str[i] != '\0' && i < 1001)
    {
        USART_Transmit(str[i]);
        i++;
    };
}
//END print_str(...)
