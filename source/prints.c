/******************************************************************************
 * Author: Joshua Fain
 * Date:   6/23/2020
 * 
 * File: PRINTS.C 
 * 
 * Requires: PRINTS.H  - header for print functions defined here.
 *           USART.H   - needed for sending commands to, and receiving
 *                       responses from, SD Card.
 *           STDINT.H  - defines data types.
 *           AVR/IO.H  - needed for I/O related AVR variables.
 * 
 * Target: ATmega 1280
 *
 * Description: 
 * Defines some print functions declared in PRINTS.H that can be used to print
 * strings and numbers in decimal, integer, and hex format.
 * 
 * Functions:
 * 1) void print_int(uint32_t n)
 * 2) void print_bin(uint32_t n)
 * 3) void print_hex(uint32_t n)
 * 4) void print_str(char str[])
 * ***************************************************************************/
 

#include <stdint.h>
#include <avr/io.h>
#include "../includes/prints.h"
#include "../includes/usart.h"


/******************************************************************************
 * Function:    print_dec(uint32_t n) 
 * Description: prints decimal value of the arguement to screen.
 * Argument(s): unsigned integer to print to screen. up to 32-bit.
 * Returns:     VOID
******************************************************************************/
void print_dec(uint32_t n)
{
    //Determine number of decimal digits required to represent the number.
    //32 bit integer can have up to 10 digits
    uint8_t len = 1;

    if(n < 10)                len = 1;
    else if(n <  100)         len = 2;
    else if(n <  1000)        len = 3;
    else if(n <  10000)       len = 4;
    else if(n <  100000)      len = 5;
    else if(n <  1000000)     len = 6;
    else if(n <  10000000)    len = 7;
    else if(n <  100000000)   len = 8;
    else if(n <  1000000000)  len = 9;
    else if(n <= 4294967295)  len = 10; //largest 32-bit integer value

    char dec[len]; //array to hold integer digit characters
    for (int i = 0; i < len; i++) dec[i] = '0'; // initialize each digit to 0.

    uint32_t result = n;
    uint8_t remainder = 0;

    for (int i = 0; i < len; i++)
    {
        remainder = result%10;
        result = result/10;

        // convert digit to ascii character
        dec[i] = remainder + 48;
    }
    
    //print characters in decimal array
    for (int i = len-1; i >= 0; i--) USART_Transmit(dec[i]); 
}
//END print_int()



/******************************************************************************
 * Function:    print_bin(uint32_t n) 
 * Description: prints value of arguement to screen as a binary integer.
 * Argument(s): unsigned integer to print. up to 32 bits.
 * Returns:     VOID
 * Notes:       prints the number as space-separated of nibbles (4bits).
 *              only prints the number of nibbles required for the number.
******************************************************************************/
void print_bin(uint32_t n)
{
    //Determine number binary digits required to represent the number.
    uint8_t len = 4;
    
    if(n < 0x10)              len = 4;
    else if(n < 0x100)        len = 8;
    else if(n < 0x1000)       len = 12;
    else if(n < 0x10000)      len = 16;
    else if(n < 0x100000)     len = 20;
    else if(n < 0x1000000)    len = 24;
    else if(n < 0x10000000)   len = 28;
    else if(n <= 0xFFFFFFFF)  len = 32;

    char bin[len]; //array to hold binary digit characters
    for (int i = 0; i < len; i++) bin[i] = '0'; //initialize each digit to 0.
    
    uint32_t result = n;
    uint8_t  remainder = 0;

    for (int i = 0; i < len; i++)
    {
        remainder = result%2;
        result = result/2;

        if (remainder == 0) 
            bin[i] = '0';
        else 
            bin[i] = '1';
    }

    //print characters in binary array. print a space every 4 bits.
    for (int i = len-1; i >= 0; i--)
    {
        USART_Transmit(bin[i]);
        if(i%4 == 0) USART_Transmit(' ');
    }
}
//END print_bin()



/******************************************************************************
 * Function:    print_hex(uint32_t n) 
 * Description: prints value of arguement to screen as a hexadecimal integer.
 * Argument(s): unsingned integer to print. up to 32 bits.
 * Returns:     VOID
******************************************************************************/
void print_hex(uint32_t n)
{
    //Determine number of hexadecimal digits required to represent the number.
    uint8_t len = 2;
    
    if (n < 0x100)           len = 2;
    else if(n < 0x1000)      len = 3;
    else if(n < 0x10000)     len = 4;
    else if(n < 0x100000)    len = 5;
    else if(n < 0x1000000)   len = 6;
    else if(n < 0x10000000)  len = 7;
    else if(n <= 0xFFFFFFFF) len = 8;

    char hex[len]; //array to hold binary digit characters
    for (int i = 0; i < len; i++) { hex[i] = '0'; } // initialize each digit to 0.
    
    uint32_t result = n;
    uint8_t remainder = 0;

    for (int i = 0; i < len; i++)
    {
        remainder = result%16;
        result = result/16;

        //convert to ascii characters.
        if(remainder < 10 && remainder >= 0) { hex[i] = remainder + 48; }  //ascii numbers
        else if(remainder >= 10 && remainder <= 16) { hex[i] = (remainder - 10) + 65; } //ascii letters
    }
    
    //print characters in hexadecimal array.
    for (int i = len-1; i >= 0; i--) USART_Transmit(hex[i]);
}    
//END print_hex()



/******************************************************************************
 * Function:    print_str(char str[]) 
 * Description: prints c-string passed in as the argument.
 * Argument(s): c-string to print to screen.
 * Returns:     VOID
 * Notes:       c-strings must be null-terminated '\0'.
 *              Currently will only print up to 1000 character string, but 
 *              haven't tested up to this length.
******************************************************************************/
void print_str(char str[])
{
    int i = 0;
    while(str[i] != '\0' && i < 1000)
    {
        USART_Transmit(str[i]);
        i++;
    };
}
//END print_str()