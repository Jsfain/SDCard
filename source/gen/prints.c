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
void print_Dec(uint32_t num)
{
  uint8_t radix = 10;             // decimal radix
  uint8_t digitCntTot = 0;        // count of totalt number of digits required
  uint8_t digit[10];             // length is maximum possible digits

  //
  // 1) Load remainder of number into array when divided by 10. 
  // 2) Update the value of the number by dividing itself by 10. 
  // 3) Repeat until number is 0. 
  // Note: The array will be loaded in reverse order.
  //
  do
  {
    digit[digitCntTot] = num % radix + 48;      // add 48 to convert to ascii
    num /= radix;
    digitCntTot++;
  }
  while (num > 0);

  // print digits
  for (int digitPos = digitCntTot - 1; digitPos >= 0; digitPos--)
    usart_Transmit(digit[digitPos]);
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
  uint8_t radix = 2;              // binary radix
  uint8_t digitCntTot = 0;        // count of total number of digits required
  uint8_t digit[32];              // length is max possible digits
  
  //
  // 1) Load remainder of number into digit array when divided by 2. 
  // 2) Update the value of the number by dividing itself by 2.
  // 4) Repeat until number is 0. 
  // Note: The array is loaded in reverse order.
  //
  do
  {
    digit[digitCntTot] = num % radix + 48;    // add 48 to convert to ascii
    num /= radix; 
    digitCntTot++;
  }
  while (num > 0);

  // print digits.
  for (int digitPos = digitCntTot - 1; digitPos >= 0; digitPos--)
  {
    usart_Transmit(digit[digitPos]); 
    if (digitPos % 4 == 0)
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
  uint8_t radix = 16;             // hex radix
  uint8_t digitCntTot = 0;        // total number of digits required
  uint8_t digit[8];               // length is max possible digits
  
  //
  // 1) Load remainder of number into digit array when divided by 16. 
  // 2) Update the value of the number by dividing itself by 16.
  // 3) Convert the value to an ascii number or letter (A-F) character.
  // 4) Repeat until number is 0. 
  // Note: The array is loaded in reverse order.
  //
  do
  {
    digit[digitCntTot] = num % radix;
    num /= radix;

    // convert to ascii characters
    if (digit[digitCntTot] < 10)
      digit[digitCntTot] += 48;   // add 48 to convert to ascii numbers
    else
      digit[digitCntTot] += 55;   // convert 10 to 15 to ascii A to F
    
    digitCntTot++;
  }
  while (num > 0);

  // print digits.
  for (int digitPos = digitCntTot - 1; digitPos >= 0; digitPos--)
    usart_Transmit(digit[digitPos]);
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
void print_Str(char* str)
{
  uint16_t charCountMax = 1000;
  for (uint16_t charCount = 0; charCount < charCountMax; charCount++)
  {
    if (str[charCount] != '\0')
      usart_Transmit(str[charCount]);
    else
      break;
  }
}

