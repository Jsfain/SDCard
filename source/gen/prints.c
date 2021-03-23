/*
 * File    : PRINTS.C
 * Version : 2.0
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : GNU GPLv3
 * Copyright (c) 2020, 2021
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
void print_Dec(uint32_t num)
{
  const uint8_t radix = 10;                 // decimal radix
  char digit[10];                           // length is max possible digits
  int  digitCnt = 0;                        // total number of digits required

  //
  // 1) Load last digit (remainder) of num into array when divided by 10.
  // 2) Update the value of the num by dividing itself by 10.
  // 4) Repeat until number is 0. 
  // Note: The array is loaded in reverse order.
  //
  for (digitCnt = 0; num > 0; digitCnt++)
  {
    digit[digitCnt] = num % radix + '0';    // add 48 to convert to ascii
    num /= radix; 
  }

  // print digits.
  if (digitCnt == 0)
    usart_Transmit('0');
  else
    for (--digitCnt; digitCnt >= 0; digitCnt--)
      usart_Transmit(digit[digitCnt]);
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
void print_Bin(uint32_t num)
{
  const uint8_t radix = 2;                  // binary radix
  char digit[32];                           // length is max possible digits
  int  digitCnt = 0;                        // total number of digits required

  //
  // 1) Load remainder of number into digit array when divided by 2. 
  // 2) Update the value of the number by dividing itself by 2.
  // 4) Repeat until number is 0. 
  // Note: The array is loaded in reverse order.
  //
  for (digitCnt = 0; num > 0; digitCnt++)
  {
    digit[digitCnt] = num % radix + '0';    // add 48 to convert to ascii
    num /= radix; 
  }

  // print digits.
  if (digitCnt == 0)
    usart_Transmit('0');
  else
    for (--digitCnt; digitCnt >= 0; digitCnt--)
    {
      usart_Transmit(digit[digitCnt]);
      // every 4 digit characters print a space
      if (digitCnt % 4 == 0)
        usart_Transmit(' ');
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
void print_Hex(uint32_t num)
{
  const uint8_t radix = 16;                 // hex radix
  char digit[8];                            // length is max possible digits
  int  digitCnt = 0;                        // total number of digits required
  
  //
  // 1) Load last digit (remainder) of num into array when divided by radix.
  // 2) Update the value of num by dividing itself by the radix.
  // 3) Convert the array value to an ascii number or letter (A-F) character.
  // 4) Repeat until num is 0. 
  // Note: The array is loaded in reverse order.
  //
  for (digitCnt = 0; num > 0; digitCnt++)
  {
    digit[digitCnt] = num % radix;
    num /= radix;

    // convert to ascii characters
    if (digit[digitCnt] < 10)
      digit[digitCnt] += '0';               // convert to ascii numbers
    else
      digit[digitCnt] += 'A' - 10;          // convert ascii A to F. Offset 10
  }

  // print digits.
  if (digitCnt == 0)
    usart_Transmit('0');
  else
    for (--digitCnt; digitCnt >= 0; digitCnt--)
      usart_Transmit(digit[digitCnt]);
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
 * Warning     : There is currently no limit on the length of the string but
 *               if the array is not null-terminiated then it will loop until
 *               without bounds, until it happens to hit a null in memory.
 * ----------------------------------------------------------------------------
 */
void print_Str(char *str)
{
  for (; *str; str++)
    usart_Transmit(*str);
}

