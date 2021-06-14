/*
 * File       : PRINTS.C
 * Version    : 3.0
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020, 2021
 * 
 * PRINST.C defines for some print functions used to print strings and unsigned
 * integers. The unsigned integers can be printed in decimal, binary, and hex 
 * formats. This is the implementation for PRINTS.H.
 */

#include <stdint.h>
#include "avr_usart.h"
#include "prints.h"

// Maximum character lengths for 32-bit numbers in different forms.
#define DEC_CHAR_LEN_MAX   10
#define HEX_CHAR_LEN_MAX   8
#define BIN_CHAR_LEN_MAX   32

//
// The digits in a binary number are printed in groups of this many chars
// which are separated by spaces. Set this value >= BIN_CHAR_LEN_MAX if no 
// spaces should be printed.
//
#define BIN_CHARS_GRP_SIZE    4           

/*
 ******************************************************************************
 *                                  FUNCTIONS 
 ******************************************************************************
 */

/*
 * ----------------------------------------------------------------------------
 *                            PRINT UNSIGNED DECIMAL (BASE-10) FORM OF INTEGERS 
 * 
 * Description : Prints unsigned decimal integer form of argument.
 * 
 * Argument    : num   - Unsigned decimal integer to be printed to screen.
 * ----------------------------------------------------------------------------
 */
void print_Dec(uint32_t num)
{
  const uint8_t radix = 10;                 // decimal radix
  char digit[DEC_CHAR_LEN_MAX];             
  int  digitCnt;                            // total number of digits required

  //
  // 1) Load last digit (remainder) of num into array when divided by radix.
  // 2) Update the value of the num by dividing itself by radix.
  // 3) Repeat until number is 0. 
  // Note: The array is loaded in reverse order.
  //
  for (digitCnt = 0; num > 0; ++digitCnt)
  {
    digit[digitCnt] = num % radix + '0';    // convert to ascii
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
 * Description : Prints the binary integer form of the argument.
 * 
 * Argument    : num   - Unsigned decimal integer to be printed to screen.
 * 
 * Notes       : 1) The function will only print the number of bits required.
 *               2) A space will be print between every BIN_CHARS_GRP_SIZE 
 *                  group of digits to make it more easily readable.
 * ----------------------------------------------------------------------------
 */
void print_Bin(uint32_t num)
{
  const uint8_t radix = 2;                  // binary radix
  char digit[BIN_CHAR_LEN_MAX];             
  int  digitCnt;                            // total number of digits required

  //
  // 1) Load remainder of num into digit array when divided by radix. 
  // 2) Update the value of the num by dividing itself by radix.
  // 3) Repeat until number is 0. 
  // Note: The array is loaded in reverse order.
  //
  for (digitCnt = 0; num > 0; digitCnt++)
  {
    digit[digitCnt] = num % radix + '0';    // convert to ascii
    num /= radix; 
  }

  // print digits.
  if (digitCnt == 0)
    usart_Transmit('0');
  else
    for (--digitCnt; digitCnt >= 0; digitCnt--)
    {
      usart_Transmit(digit[digitCnt]);
      if (digitCnt % BIN_CHARS_GRP_SIZE == 0)         // print a space ?
        usart_Transmit(' ');
    }
}

/*
 * ----------------------------------------------------------------------------
 *                                         PRINT HEXADECIMAL FORM OF AN INTEGER
 *                                       
 * Description : Prints the hexadecimal form of the argument.
 * 
 * Argument    : num   - Unsigned decimal integer to be printed to screen.
 * ----------------------------------------------------------------------------
 */
void print_Hex(uint32_t num)
{
  const uint8_t radix = 16;                 // hex radix
  char digit[HEX_CHAR_LEN_MAX];             
  int  digitCnt;                            // total number of digits required
  
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
 * Argument    : str   - Pointer to a null-terminated char array (i.e. string)
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

