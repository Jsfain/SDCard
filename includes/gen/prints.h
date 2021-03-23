/*
 * File    : PRINTS.H
 * Version : 2.0
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : GNU GPLv3
 * Copyright (c) 2020, 2021
 * 
 * Interface for some print functions used to print strings and unsigned
 * integers in decimal, binary, and hex formats.
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
 *                                    PRINT UNSIGNED DECIMAL (BASE-10) INTEGERS 
 * 
 * Description : Prints unsigned decimal integer form of argument to screen.
 * 
 * Arguments   : num     Unsigned integer to be printed to the screen.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */
void print_Dec(uint32_t num);

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
void print_Bin(uint32_t num);

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
void print_Hex(uint32_t num);

/*
 * ----------------------------------------------------------------------------
 *                                                               PRINT C-STRING
 *                                       
 * Description : Prints the C-string passed as the argument.
 * 
 * Argument    : str     Pointer to a null-terminated char array (i.e. 
 *                       c-string), that will be printed to the screen. 
 * 
 * Notes       : Strings up to 999 characters + '\0' (null character) can
 *               currently be handled by this function.
 * ----------------------------------------------------------------------------
 */
void print_Str(char *str);

#endif //PRINTS_H
