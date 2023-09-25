/*
 * File       : PRINTS.H
 * Version    : 3.0
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020 - 2023
 * 
 * Description: Interface for some print functions. Used to print unsigned 
 *              integers (decimal, binary, hex) and strings.
 */

#ifndef PRINTS_H
#define PRINTS_H

/*
 ******************************************************************************
 *                                 MACROS 
 ******************************************************************************
 */

// Maximum character lengths for 32-bit numbers in different forms.
#define DEC_CHAR_LEN_MAX   10
#define HEX_CHAR_LEN_MAX   8
#define BIN_CHAR_LEN_MAX   32

//
// Binary digits will be printed in space-separated groups of this many chars.
// Set this value >= BIN_CHAR_LEN_MAX if no spaces should be printed.
//
#define BIN_CHARS_GRP_SIZE    4    

/*
 ******************************************************************************
 *                            FUNCTION PROTOTYPES   
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
void print_Dec(uint32_t num);

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
void print_Bin(uint32_t num);

/*
 * ----------------------------------------------------------------------------
 *                                         PRINT HEXADECIMAL FORM OF AN INTEGER
 *                                       
 * Description : Prints the hexadecimal form of the argument.
 * 
 * Argument    : num   - Unsigned decimal integer to be printed to screen.
 * ----------------------------------------------------------------------------
 */
void print_Hex(uint32_t num);

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
void print_Str(char *str);

#endif //PRINTS_H
