/*
***********************************************************************************************************************
*                                                   AVR-GENERAL MODULE
*
* File    : USART.H
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
*
*
* DESCRIPTION:
* Declares USART related functions as well as defines CPU frequency and baud rate MACROS. 
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

#ifndef USART_H
#define USART_H


/******************************************************************************
 * Definition:  F_CPU
 * Description: specifies the frequency of the clock source.
******************************************************************************/
#define F_CPU   16000000UL



/******************************************************************************
 * Definition:  BAUD
 * Description: defines the decimal-based baud rate to be used by USART0.
******************************************************************************/
#define BAUD    9600


/******************************************************************************
 * Function:    UBRR_VALUE
 * Description: calculates the value to load into UBRR (UBRRH/UBRRL) for the 
 *              baud rate specified by BAUD. See ATmege datasheet for formula.
 * Argument(s): F_CPU, BAUD
 * Returns:     Value to be loaded into UBRR
******************************************************************************/
#define UBRR_VALUE  (F_CPU/16/BAUD) - 1 


/******************************************************************************
 * Functions: Declaration of standard USART initialization, transmit, and 
 *            receive functions. See USART.C for detailed descriptions of each.
******************************************************************************/

//Initializes USART0 for target device.
void USART_Init();


//Function for receiving data via USART0 initialized by USART_Init().
uint8_t USART_Receive();


//Function for transmitting data via USART0 initialized by USART_Init().
void USART_Transmit(uint8_t data);



#endif //USART_H