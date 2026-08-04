/*
 * File       : PRINTS.C
 * Version    : 0.1
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020 - 2026
 * 
 * Description: Implements PRINTS.H. These functions can be used to print 
 *              numbers (unsigned integers), characters, and C-strings. 
 *              Numbers can be printed in either decimal, binary, or hex forms
 *              using the print_Num() function and specifying the base.
 * 
 * Out Stream : Each print function must also be passed a pointer to a user-
 *              defined output stream function. This function must be capable
 *              of accepting a single 8-bit value and should direct it to the
 *              the intended target device's output port, e.g. USART port. The 
 *              only requirement for this function in PRINTS is that it accepts 
 *              a single 8-bit value as it's only argument. Where the value is 
 *              directed is up to the user-defined function.
 */

#include <stdint.h>
#include "prints.h"

/*
 ******************************************************************************
 *                         "PRIVATE" FUNCTION PROTOTYPES
 ******************************************************************************
 */

 /* 
  *  These internal functions are used to translate numbers into their desired 
  *  format, i.e. decimal, binary, or hex 
  */
static void pvt_print_Dec(uint32_t num, void (*outs)(uint8_t));
static void pvt_print_Bin(uint32_t num, void (*outs)(uint8_t));
static void pvt_print_Hex(uint32_t num, void (*outs)(uint8_t));


/*
 ******************************************************************************
 *                            FUNCTIONS 
 ******************************************************************************
 */

/*
 * ----------------------------------------------------------------------------
 *                                          PRINT CHAR (unsigned 8-bit integer) 
 * 
 * Description : Prints a single 8-bit character. This function also acts as 
 *               the interface for the target's selected output (e.g USART). 
 *               All other functions in this file call this function to 
 *               transmit their characters to the output port.
 * 
 * Argument    : val   - Char (Unsigned integer) to be printed.
 * ----------------------------------------------------------------------------
 */
void print_Char(uint8_t val, void (*outs)(uint8_t))
{
  outs(val);  // call user-defined I/O transmit function
}

/*
 * ----------------------------------------------------------------------------
 *                                                  PRINT NUMBER (UNSIGNED INT) 
 * 
 * Description : Prints unsigned integer in the form specified by base.
 * 
 * Argument    : num   - Unsigned integer to be printed to screen.
 *               base  - Base to print the number. Base options are binary (2),
 *                       decimal (10), or hex (16).
 * 
 * Notes.      : 1) For hex or binary forms, the function only prints the 
 *                  number of digits required to represent the number.
 *               2) For binary readability, a space is printed by default 
 *                  between every 4 digits. This can be changed by setting the
 *                  BIN_CHARS_GRP_SIZE.
 *               3) Hex numbers are printed with the 0x prefix.
 * ----------------------------------------------------------------------------
 */
void print_Num(uint32_t num, uint8_t base, void (*outs)(uint8_t))
{
  switch (base)
  {
    case 2:
      pvt_print_Bin(num, outs);
      break;
    case 10:
      pvt_print_Dec(num, outs);
      break;
    case 16:
      pvt_print_Hex(num, outs);
      break;
    default:
      print_Str("Error: Invalid base. Must be 2, 10, or 16.", outs);
      break;
  }
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
 * Warning     : There is currently no limit on the length of the string, but
 *               if the array is not null-terminiated then it will loop
 *               continuously until it happens to hit a null in memory.
 * ----------------------------------------------------------------------------
 */
void print_Str(char *str, void (*outs)(uint8_t))
{
  for (; *str; str++)
    print_Char(*str, outs);
}


/*
 ******************************************************************************
 *                         "PRIVATE" FUNCTION PROTOTYPES
 ******************************************************************************
 */

/*
 * ----------------------------------------------------------------------------
 *                             PRINT UNSIGNED DECIMAL (BASE-10) FORM OF INTEGER 
 * 
 * Description : Prints unsigned decimal integer form of argument.
 * 
 * Argument    : num   - Unsigned decimal integer to be printed to screen.
 * ----------------------------------------------------------------------------
 */
static void pvt_print_Dec(uint32_t num, void (*outs)(uint8_t))
{
  const uint8_t radix = 10;                 // decimal radix
  char digit[DEC_CHAR_LEN_MAX];             
  int  digitCnt;                            // total number of digits required

  //
  // 1) Load last digit (remainder) of num into array when divided by radix.
  // 2) Update the value of num by dividing itself by radix.
  // 3) Repeat until number is 0. 
  // Note: The array is loaded in reverse order.
  //
  for (digitCnt = 0; num > 0; ++digitCnt)
  {
    digit[digitCnt] = num % radix + '0';    // convert to ascii
    num /= radix; 
  }

  // print digits
  if (digitCnt == 0)
    print_Char('0', outs);
  else
    for (--digitCnt; digitCnt >= 0; digitCnt--)
      print_Char(digit[digitCnt], outs);
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
 *               2) A space will be printed between every BIN_CHARS_GRP_SIZE 
 *                  group of digits to make it more easily readable.
 * ----------------------------------------------------------------------------
 */
static void pvt_print_Bin(uint32_t num, void (*outs)(uint8_t))
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
    print_Char('0', outs);
  else
    for (--digitCnt; digitCnt >= 0; digitCnt--)
    {
      print_Char(digit[digitCnt], outs);
      if (BIN_CHARS_GRP_SIZE > 0 && digitCnt % BIN_CHARS_GRP_SIZE == 0)   // if group size is 0 then no spaces
      //if (digitCnt % BIN_CHARS_GRP_SIZE == 0)         // print a space
        print_Char(' ', outs);
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
static void pvt_print_Hex(uint32_t num, void (*outs)(uint8_t))
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
    print_Char('0', outs);
  else
    for (--digitCnt; digitCnt >= 0; digitCnt--)
      print_Char(digit[digitCnt], outs);
}
