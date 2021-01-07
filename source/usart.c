/*
*******************************************************************************
*                                AVR-GENERAL MODULE
*
* File    : USART.C
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
* License : MIT
* Copyright (c) 2020
* 
*
* DESCRIPTION:
* Functions for interacting with the ATMega's USART0 port.
*******************************************************************************
*/


#include <stdint.h>
#include <avr/io.h>
#include "../includes/usart.h"





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
usart_init (void)
{
    //Set baud rate
    UBRR0H = (uint8_t)((UBRR_VALUE) >> 8);
    UBRR0L = (uint8_t)(UBRR_VALUE);

    // Enable USART0 receiver and transmitter
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    
    // Set USART in Asynch mode, no parity, data frame = 8 data, 1 stop bits
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}



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
usart_receive (void)
{
    // Wait for data to be received by polling the receive complete flag
    while ( !(UCSR0A & (1 << RXC0))) ;
    
    //return byte received in buffer
    return UDR0; 
}



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
usart_transmit (uint8_t data)
{
    //Wait for transmit buffer to be empty by polling Data Register Empty flag
    while( !( UCSR0A & (1 << UDRE0)) ) ;
    
    //Place data byte into buffer to be transmitted via USART
    UDR0 = data;
}