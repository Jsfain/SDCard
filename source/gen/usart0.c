/*
 * File    : USART0.C
 * Version : 0.0.0.1 
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : MIT
 * Copyright (c) 2020
 * 
 * Implementation of USART.H
 */

#include <stdint.h>
#include <avr/io.h>
#include "usart0.h"

/*
 ******************************************************************************
 *                                  FUNCTIONS
 ******************************************************************************
 */

/*
 * ----------------------------------------------------------------------------
 *                                                             INITIALIZE USART
 *                                        
 * Description : Initializes USART0 of the ATMega target device.
 * 
 * Arguments   : void 
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */
void usart_Init(void)
{
  // Set baud rate
  UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
  UBRR0L = (uint8_t)UBRR_VALUE;

  // Enable USART0 receiver and transmitter
  UCSR0B = 1 << RXEN0 | 1 << TXEN0;
  
  // Set USART - Asynch mode, no parity, data frame = 8 data, 1 stop
  UCSR0C = 1 << UCSZ01 | 1 << UCSZ00;
}

/*
 * ----------------------------------------------------------------------------
 *                                                           USART RECEIVE BYTE
 *                                         
 * Description : Receives a byte using the USART0 on the ATmega target device.
 * 
 * Arguments   : void
 * 
 * Returns     : byte received by the USART0, i.e. value in UDR0.
 * ----------------------------------------------------------------------------
*/
uint8_t usart_Receive(void)
{
  // poll the RX complete flag, until it is set
  while ( !(UCSR0A & 1 << RXC0))
    ;
  
  // return byte received into usart buffer
  return UDR0; 
}

/*
 * ----------------------------------------------------------------------------
 *                                                          USART TRANSMIT BYTE
 *                                       
 * Description : Sends a byte to another device via the USART0.
 * 
 * Arguments   : data     byte to sent via USART0.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */
void usart_Transmit(const uint8_t data)
{
  // poll Data Reg Empty Flag until it is set.
  while( !(UCSR0A & 1 << UDRE0))
    ;
  
  // load data into usart buffer which will transmit it.
  UDR0 = data;
}
