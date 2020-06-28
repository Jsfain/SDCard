/******************************************************************************
 * Author: Joshua Fain
 * Date:   6/23/2020
 * 
 * File: PRINTS.H 
 * 
 * Required by: PRINTS.C 
 * 
 * Target: ATmega 1280
 *
 * Description: 
 * Declares some print functions that can be used to print strings as well as
 * numbers in decimal, integer, and hex formats.
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