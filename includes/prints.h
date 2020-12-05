/*
***********************************************************************************************************************
*                                                   AVR-GENERAL MODULE
*
* File    : PRINTS.H
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
*
*
* DESCRIPTION:
* Declare some print functions used to print strings and positive integers in decimal, binary, and hex formats.
*
*
* FUNCTIONS:
*   (1) void print_dec (uint32_t num)
*   (2) void print_bin (uint32_t num)
*   (3) void print_hex (uint32_t num)
*   (4) void print_str (char * str)
*                                                 
*                                                       MIT LICENSE
*
* Copyright (c) 2020 Joshua Fain
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
* documentation files (the "Software"), to deal in the Software without restriction, including without limitation the 
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to 
* permit ersons to whom the Software is furnished to do so, subject to the following conditions: The above copyright 
* notice and this permission notice shall be included in all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***********************************************************************************************************************
*/


#ifndef PRINTS_H
#define PRINTS_H


/*
***********************************************************************************************************************
*                                                       FUNCTIONS
***********************************************************************************************************************
*/

/*
***********************************************************************************************************************
 *                                        PRINT POSITIVE DECIMAL (BASE-10) INTEGERS
 * 
 * Description : Prints unsigned (positive) base-10 (decimal) integers to a terminal.
 *  
 * Argument    : n   Unsigned integer to be printed to the terminal/screen in decimal form.
***********************************************************************************************************************
*/

void 
print_dec(uint32_t num);



/*
***********************************************************************************************************************
 *                                            PRINT BINARY FORM OF INTEGER
 * 
 * Description : Prints the binary form of the unsigned integer argument.
 *  
 * Argument    : n   Unsigned integer to be printed to the terminal/screen in binary form.
 * 
 * Notes       : This function prints the number as space-separated nibbles (4bits), and only the number of required
 *               nibbles will be printed. required.
***********************************************************************************************************************
*/

void 
print_bin(uint32_t num);



/*
***********************************************************************************************************************
 *                                        PRINT HEXADECIMAL FORM OF AN INTEGER
 * 
 * Description : Prints the hexadecimal form of the unsigned integer argument.
 *  
 * Argument    : n   Unsigned integer to be printed to the terminal/screen in hexadecimal form.
***********************************************************************************************************************
*/

void 
print_hex(uint32_t num);



/*
***********************************************************************************************************************
 *                                                  PRINT C-STRING
 * 
 * Description : Prints the C-string passed as the argument.
 *  
 * Argument    : str   Pointer to C-String char array that will be printed to the terminal. 
 * 
 * Notes:      : (1)  C-strings are null-terminated '\0', and therefore this function will only work if a char array
 *                    contains a NULL. If it doesn't then unexpected results will likely occur.
 *               (2)  Only strings of 999 characters + NULL are currently handled by this function.
***********************************************************************************************************************
*/

void 
print_str(char * str);

#endif //PRINTS_H
