/*
*******************************************************************************
*                                AVR-GENERAL MODULE
*
* File    : USART.H
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
* License : MIT
* Copyright (c) 2020
* 
*
* DESCRIPTION:
* Interface for interacting with the ATMega's USART0 port.
*******************************************************************************
*/


#ifndef USART_H
#define USART_H





/*
*******************************************************************************
*******************************************************************************
 *                     
 *                                    MACROS
 *  
*******************************************************************************
*******************************************************************************
*/

// Specify clock frequency of target device.
#define F_CPU   16000000UL



// Define decimal-based baud rate to be used by USART0.
#define BAUD    9600


// Calculate the value to load into UBRR (UBRRH & UBRRL) 
// for the decimal baud rate specified by BAUD.
#define UBRR_VALUE  (F_CPU/16/BAUD) - 1 





/*
*******************************************************************************
*******************************************************************************
 *                     
 *                                  FUNCTIONS
 *  
*******************************************************************************
*******************************************************************************
*/

/*
-------------------------------------------------------------------------------
|                                INITIALIZE USART
|                                        
| Description : Initializes USART0 of the ATMega target device.
-------------------------------------------------------------------------------
*/


void 
usart_init (void);



/*
-------------------------------------------------------------------------------
|                             RECEIVE BYTE VIA USART
|                                        
| Description : Receives a byte using the USART0 on the ATmega target device.
|
| Returns     : byte received into UDR0 via USART0.
-------------------------------------------------------------------------------
*/

uint8_t 
usart_receive (void);



/*
-------------------------------------------------------------------------------
|                            TRANSMIT BYTE VIA USART
|                                        
| Description : Sends a byte to another device via the USART0.
|
| Arguments   : byte to send via USART0.
-------------------------------------------------------------------------------
*/

void 
usart_transmit (uint8_t data);



#endif //USART_H