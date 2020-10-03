/******************************************************************************
 * Copyright (c) 2020 Joshua Fain
 * 
 * 
 * USART.H 
 * 
 * 
 * DESCRIPTION
 * Declares USART related functions as well as defines CPU frequency and baud. 
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
******************************************************************************/


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