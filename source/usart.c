/*
***********************************************************************************************************************
*                                                   AVR-GENERAL MODULE
*
* File    : USART.C
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
*
*
* DESCRIPTION:
* Defines standard functions declared in USART.H used for initializing, sending, and recieving data via USART0 on 
* the target device. Most of these are essentially the definitions found in the ATmega datasheet.
*
*
* FUNCTIONS:
*   (1) void    USART_init (void)
*   (2) uint8_t USART_receive (void)
*   (3) void    USART_transmit (uint8_t data)
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



#include <stdint.h>
#include <avr/io.h>
#include "../includes/usart.h"



/*
***********************************************************************************************************************
 *                                                   INITIALIZE USART
 * 
 * Description  : Initializes USART0 of target device by setting the baud rate, enabling transmit and receive and 
 *                defining data frame format of 8 data bits and 1 stop bit.
***********************************************************************************************************************
*/

void 
USART_init (void)
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
***********************************************************************************************************************
 *                                               RECEIVE BYTE VIA USART
 * 
 * Description  : Standard USART receive function used to get the byte received in USART0 by polling the RXC0 flag and
 *                then returning the byte in UDR0.
 * 
 * Argument     : void
 * 
 * Returns      : data byte received in UDR0.
***********************************************************************************************************************
*/

uint8_t 
USART_receive (void)
{
    // Wait for data to be received by polling the receive complete flag
    while ( !(UCSR0A & (1 << RXC0))) ;
    
    //return byte received in buffer
    return UDR0; 
}



/*
***********************************************************************************************************************
 *                                               TRANSMIT BYTE VIA USART
 * 
 * Description  : Standard USART transmit function used to transmit a data byte by polling the UDRE0 flag. When the 
 *                flag is set, UDR0 is empty and a new data byte can be loaded into UDR0 to be sent by USART. 
 * 
 * Argument     : byte    Unsigned data byte to send via USART.
***********************************************************************************************************************
*/

void 
USART_transmit (uint8_t data)
{
    //Wait for transmit buffer to be empty by polling Data Register Empty flag
    while( !( UCSR0A & (1 << UDRE0)) ) ;
    
    //Place data byte into buffer to be transmitted via USART
    UDR0 = data;
}