/*************************************************************************
 * Copyright (c) 2020 Joshua Fain
 * 
 * 
 * USART.C
 * 
 * 
 * DESCRIPTION
 * Defines standard functions declared in USART.H used for initializing,
 * sending, and recieving data via USART0 on target device. These are 
 * mostly the same as those found in the ATmega datasheet.
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
 * **********************************************************************/


#include <stdint.h>
#include <avr/io.h>
#include "../includes/usart.h"


/******************************************************************************
 * Function:    USART_Init()
 * Description: Initializes USART0 of target device by setting baud rate,
 *              enabling transmit and receive and defining data frame format of
 *              8 data bits and 1 stop bit
 * 
 * Argument(s): VOID
 * Returns:     VOID
******************************************************************************/
void USART_Init(void)
{
    //Set baud rate
    UBRR0H = (uint8_t)((UBRR_VALUE)>>8);
    UBRR0L = (uint8_t)(UBRR_VALUE);

    // Enable USART0 receiver and transmitter
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    
    // Set USART in Asynch mode, no parity, data frame = 8 data, 1 stop bits
    UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
}


/******************************************************************************
 * Function:    USART_Receive(void)
 * Description: standard USART receive function used to get the byte received
 *              in USART0 by polling the RXC0 flag and then returning the byte
 *              in UDR0.
 * Argument(s): VOID
 * Returns:     8-bit data byte received in UDR0
******************************************************************************/
uint8_t USART_Receive(void)
{
    // Wait for data to be received by polling the receive complete flag
    while( !(UCSR0A & (1<<RXC0)) ) ;
    
    //return byte received in buffer
    return UDR0; 
}


/******************************************************************************
 * Function:    USART_Transmit(uint8_t data)
 * Description: standard USART transmit function used to transmit a data byte
 *              by polling the UDRE0 flag. When flag is set, UDR0 is empty 
 *              and new data byte can be loaded into UDR0 to be sent by USART. 
 * Argument(s): 8-bit data byte.
 * Returns:     VOID
******************************************************************************/
void USART_Transmit(uint8_t data)
{
    //Wait for transmit buffer to be empty by polling Data Register Empty flag
    while( !( UCSR0A & (1<<UDRE0)) ) ;
    
    //Place data byte into buffer to be transmitted via USART
    UDR0 = data;
}