/*
 * File    : PRINTS.C
 * Version : 0.0.0.2
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : MIT
 * Copyright (c) 2020
 * 
 * Implementation of PRINTS.H
 */

#include <stdint.h>
#include <avr/io.h>
#include "prints.h"
#include "usart0.h"


/*
 ******************************************************************************
 *                                  FUNCTIONS 
 ******************************************************************************
 */

/*
 * ----------------------------------------------------------------------------
 *                                    PRINT UNSIGNED DECIMAL (BASE-10) INTEGERS 
 * 
 * Description : Prints unsigned decimal integer form of argument to screen.
 * 
 * Arguments   : num     Unsigned integer to be printed to the screen.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */

void print_dec (uint32_t num)
{
  uint8_t cnt = 0;                // count for number of digits required
  char    arr[10];                // array length is maximum possible digits

  //
  // 1) Load remainder of number into array when divided by 10. 
  // 2) Update the value of the number by dividing itself by 10. 
  // 3) Repeat until number is 0. 
  // Note: The array will be loaded in reverse order.
  //
  do
  {
    arr[cnt] = num%10 + 48;       // add 48 to convert to ascii
    num /= 10;
    cnt++;  
  }
  while (num > 0);

  // print digits. 
  for (int i = cnt-1; i >= 0; i--)
    usart_transmit (arr[i]);
}


/*
 * ----------------------------------------------------------------------------
 *                                                 PRINT BINARY FORM OF INTEGER 
 *                                        
 * Description : Prints the binary form of the integer argument to the screen.
 * 
 * Argument    : num     Unsigned integer to be printed to the screen.
 * 
 * Returns     : void 
 * 
 * Notes       : 1) The function will only print the number of bits required.
 *               2) A space will be print between every 4-bit group.
 * ----------------------------------------------------------------------------
 */

void print_bin (uint32_t num)
{
  uint8_t cnt = 0;                // count for number of digits required
  char    arr[32];                // array length is maximum possible digits

  //
  // 1) Load remainder of number into array when divided by 2. 
  // 2) Update the value of the number by dividing itself by 2. 
  // 3) Repeat until number is 0. 
  // Note: The array will be loaded in reverse order.
  //
  do
  {
    arr[cnt] = num%2 + 48;        // add 48 to convert to ascii
    num /= 2;
    cnt++;
  }
  while (num > 0);

  // print digits
  for (int i = cnt-1; i >= 0; i--)
  {
    usart_transmit (arr[i]);
    // print space every 4 digits
    if (i%4 == 0) 
      usart_transmit (' ');
  }    
}


/*
 * ----------------------------------------------------------------------------
 *                                         PRINT HEXADECIMAL FORM OF AN INTEGER
 *                                       
 * Description : Prints the hexadecimal form of unsigned integer argument to
 *               the screen.
 * 
 * Argument    : num     Unsigned integer to be printed to the screen.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */

void print_hex (uint32_t num)
{
  uint8_t cnt = 0;                // count for number of digits required
  char    arr[8];                 // array length is maximum possible digits
       
  //
  // 1) Load remainder of number into array when divided by 16. 
  // 2) Update the value of the number by dividing itself by 16.
  // 3) Convert the value to an ascii number or letter A-F.
  // 4) Repeat until number is 0. 
  // Note: The array will be loaded in reverse order.
  //
  do
  {
      arr[cnt] = num % 16;
      num /= 16;

      // convert to ascii characters
      if (arr[cnt] < 10)
        arr[cnt] += 48;         // ascii numbers
      else
        arr[cnt] += 55;         // convert 10-15 to ascii A-F
      
      cnt++;
  }
  while (num > 0);

  //print digits.
  for (int i = cnt-1; i >= 0; i--) 
    usart_transmit (arr[i]);
}    


/*
 * ----------------------------------------------------------------------------
 *                                                               PRINT C-STRING
 *                                       
 * Description : Prints the C-string passed as the argument.
 * 
 * Argument    : str     Pointer to a null-terminated char array (i.e. string)
 *                       that will be printed to the screen.
 * 
 * Notes       : Strings up to 999 characters + '\0' (null character) can
 *               currently be handled by this function.
 * ----------------------------------------------------------------------------
 */

void print_str (char * str)
{
  uint16_t cnt = 0;
  while (str[cnt] != '\0' && cnt <= 1000)
  {
    usart_transmit (str[cnt]);
    cnt++;
  };
}

