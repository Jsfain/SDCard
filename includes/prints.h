/******************************************************************************
 * Copyright (c) 2020 Joshua Fain
 * 
 * 
 * PRINTS.H 
 * 
 *
 * DESCRIPTION
 * Declares some print functions that can be used to print strings as well as
 * numbers in decimal, integer, and hex formats.
 * 
 * 
 * TARGET
 * ATmega 1280 
 * 
 * 
 * VERSION
 * 0.0.0.1
 * 
 *
 * LICENSE
 * Licensed under the GNU GPL v3
 * ***************************************************************************/


#ifndef PRINTS_H
#define PRINTS_H


/******************************************************************************
 * Functions: Declaration of print functions. See PRINTS.C for details.
******************************************************************************/

//prints decimal integer up to 32 bits
void print_dec(uint32_t);

//prints binary integer up to 32 bits
void print_bin(uint32_t);

//prints hexadecimal integer up to 32 bits
void print_hex(uint32_t);

//prints c-string up to 1000 characters long
void print_str(char str[]);

#endif //PRINTS_H