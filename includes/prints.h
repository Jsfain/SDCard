/*
***********************************************************************************************************************
*                                                   AVR-GENERAL MODULE
*
* File    : PRINTS.H
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
*
* DESCRIPTION:
* Declare some print functions used to print strings and positive integers in decimal, binary, and hex formats.
*
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


/******************************************************************************
 * Functions: Declaration of print functions. See PRINTS.C for details.
******************************************************************************/

//prints unsigned (positive) base-10 (decimal) integer up to 32 bits
void print_dec(uint32_t);

//prints binary integer up to 32 bits
void print_bin(uint32_t);

//prints hexadecimal integer up to 32 bits
void print_hex(uint32_t);

//prints c-string up to 1000 characters long
void print_str(char str[]);

#endif //PRINTS_H
