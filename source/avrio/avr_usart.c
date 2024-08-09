/*
 * File       : AVR_USART.C
 * Version    : 1.0 
 * Target     : ATMega1280
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020 - 2023
 * 
 * Description:  Implements AVR_USART.H for accessing and controlling the USART
 *               on an AVR microcontroller.
 */

#include <avr/io.h>
#include "avr_usart.h"

/*
 ******************************************************************************
 *                                  FUNCTIONS
 ******************************************************************************
 */

/*
 * ----------------------------------------------------------------------------
 *                                                             INITIALIZE USART
 *                                        
 * Description : Initialize USART on the AVR target device.
 * ----------------------------------------------------------------------------
 */
void usart_Init(void)
{
  // Set baud rate. See BAUD in header file for value. 
  UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
  UBRR0L = (uint8_t)UBRR_VALUE;

  // Enable USARTs receiver and transmitter
  UCSR0B = 1 << RXEN0 | 1 << TXEN0;
  
  // Set USART - Asynch mode, no parity, data frame = 8 data and 1 stop bit
  UCSR0C = 1 << UCSZ01 | 1 << UCSZ00;
}

/*
 * ----------------------------------------------------------------------------
 *                                                                USART RECEIVE
 *                                         
 * Description : Receive a character via USART on the AVR target device.
 * 
 * Returns     : character received by the USART, i.e. value in UDR0.
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
 *                                                               USART TRANSMIT
 *                                       
 * Description : Sends a character via USART.
 * 
 * Arguments   : data - data to send via USART.
 * ----------------------------------------------------------------------------
 */
void usart_Transmit(uint8_t data)
{
  // poll Data Reg Empty Flag until it is set.
  while( !(UCSR0A & 1 << UDRE0))
    ;
  
  // load data into usart buffer to transmit it.
  UDR0 = data;
}
