/*
 * File       : PRINTS.H
 * Version    : 3.0
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020, 2021
 * 
 * PRINTS.H provides an interface for some print functions used to print 
 * strings and unsigned integers. The unsigned integers can be printed in 
 * decimal, binary, and hex formats.
 */

#ifndef PRINTS_H
#define PRINTS_H

/*
 ******************************************************************************
 *                            FUNCTION PROTOTYPES   
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
 *               2) A space will be print between every BIN_CHARS_GRP_SIZE 
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
 * Warning     : There is currently no limit on the length of the string but
 *               if the array is not null-terminiated then it will loop until
 *               without bounds, until it happens to hit a null in memory.
 * ----------------------------------------------------------------------------
 */
void print_Str(char *str);

#endif //PRINTS_H
