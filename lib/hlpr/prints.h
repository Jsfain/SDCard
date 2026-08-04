/*
 * File       : PRINTS.H
 * Version    : 0.1
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020 - 2026
 *
 * Description: Interface for functions to print numbers (unsigned integers),
 *              characters, and C-strings. Numbers can be printed in either
 *              decimal, binary, or hex forms using the print_Num() function 
 *              and specifying the base, 10, 2, or 16, respectively.
 * 
 * Out Stream : Each print function must also be passed a pointer to a user-
 *              defined output stream function. This function must be capable
 *              of accepting a single 8-bit value and should direct it to the
 *              the intended target device's output port, e.g. USART port. The 
 *              only requirement for this function in PRINTS is that it accepts 
 *              a single 8-bit value as it's only argument. Where the value is 
 *              directed is up to the user-defined function.
 */

#ifndef PRINTS_H
#define PRINTS_H

/*
 ******************************************************************************
 *                                 MACROS 
 ******************************************************************************
 */

// Maximum character lengths for 32-bit integers for different bases.
#define DEC_CHAR_LEN_MAX   10
#define HEX_CHAR_LEN_MAX   8
#define BIN_CHAR_LEN_MAX   32

//
// To improve readability of large binary numbers, their digits can be printed
// in groups separated by a single space. The digit group size is set by this 
// value. Default is 4.
//
#define BIN_CHARS_GRP_SIZE    4

/*
 ******************************************************************************
 *                            FUNCTION PROTOTYPES   
 ******************************************************************************
 */

/*
 * ----------------------------------------------------------------------------
 *                                          PRINT CHAR (unsigned 8-bit integer) 
 * 
 * Description : Prints a single 8-bit character. All other functions in 
 *               PRINTS.H/C call this function to place each character on the 
 *               "output stream" as such this function is the interface between
 *               all print functions and the output stream.
 * 
 * Argument    : val   - Char/Unsigned Integer to be printed.
 *               *outs - Pointer to the user-defined output stream function.
 * ----------------------------------------------------------------------------
 */
void print_Char(uint8_t val, void (*outs)(uint8_t));

/*
 * ----------------------------------------------------------------------------
 *                                                               PRINT C-STRING
 *                                       
 * Description : Prints the C-string passed as the argument.
 * 
 * Argument    : str   - Pointer to a null-terminated char array (i.e. string)
 *                       that will be printed to the screen.
 *               *outs - Pointer to the user-defined output stream function.
 * 
 * Warning     : There is currently no limit on the length of the string, but
 *               if the array is not null-terminiated then it will loop
 *               continuously until it happens to hit a null in memory.
 * ----------------------------------------------------------------------------
 */
void print_Str(char *str, void (*outs)(uint8_t));

/*
 * ----------------------------------------------------------------------------
 *                                                  PRINT NUMBER (UNSIGNED INT) 
 * 
 * Description : Prints unsigned integer in the form specified by base.
 * 
 * Argument    : num   - Unsigned integer to be printed to screen.
 *               base  - Base to print the number. Base options are binary (2),
 *                       decimal (10), or hex (16).
 *               *outs - Pointer to the user-defined output stream function.
 * 
 * Notes.      : 1) For hex or binary forms, the function only prints the 
 *                  number of digits required to represent the number.
 *               2) For binary readability, a space is printed by default 
 *                  between every 4 digits. This can be changed by setting the
 *                  BIN_CHARS_GRP_SIZE.
 *               3) Hex numbers are printed with the 0x prefix.
 * ----------------------------------------------------------------------------
 */
void print_Num(uint32_t num, uint8_t base, void (*outs)(uint8_t));

#endif //PRINTS_H
