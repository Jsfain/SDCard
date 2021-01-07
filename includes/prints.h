/*
*******************************************************************************
*                                  AVR-FAT MODULE
*
* File    : PRINTS.H
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
* License : MIT
* Copyright (c) 2020
* 
*
* DESCRIPTION:
* Interface for some print functions used to print strings and positive
* integers in decimal, binary, and hex formats.
*******************************************************************************
*/

#ifndef PRINTS_H
#define PRINTS_H





/*
*******************************************************************************
*******************************************************************************
 *                     
 *                           FUNCTION DECLARATIONS       
 *  
*******************************************************************************
*******************************************************************************
*/

/*
-------------------------------------------------------------------------------
|                  PRINT POSITIVE DECIMAL (BASE-10) INTEGERS 
|                                        
| Description : Prints unsigned base-10 integers to a screen.
|
| Argument    : num    - Unsigned integer to be printed to the screen.
-------------------------------------------------------------------------------
*/

void 
print_dec (uint32_t num);



/*
-------------------------------------------------------------------------------
|                           PRINT BINARY FORM OF INTEGER 
|                                        
| Description : Prints the binary form of the unsigned integer argument.
|
| Argument    : num    - Unsigned integer to be printed to the screen.
|
| Notes       : Prints the number as space-separated nibbles (4bits), and only 
|               the number of required nibbles will be printed.
-------------------------------------------------------------------------------
*/

void 
print_bin (uint32_t num);



/*
-------------------------------------------------------------------------------
|                      PRINT HEXADECIMAL FORM OF AN INTEGER
|                                        
| Description : Prints the hexadecimal form of the unsigned integer argument.
|
| Argument    : num    - Unsigned integer to be printed to the screen.
-------------------------------------------------------------------------------
*/

void 
print_hex (uint32_t num);



/*
-------------------------------------------------------------------------------
|                                 PRINT C-STRING
|                                        
| Description : Prints the C-string passed as the argument.
|
| Argument    : str  - ptr to the string that will be printed to the terminal. 
|
| Notes:      : (1)  C-strings are null-terminated '\0'.
|               (2)  Only strings of 999 characters + NULL are currently
|                    handled by this function.
-------------------------------------------------------------------------------
*/

void 
print_str (char * str);



#endif //PRINTS_H
