/*
 * File    : USART0.H
 * Version : 0.0.0.1 
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : MIT
 * Copyright (c) 2020
 * 
 * Interface for interacting with the ATMega's USART0 port.
 */

#ifndef USART0_H
#define USART0_H


/*
 ******************************************************************************
 *                                  MACROS
 ******************************************************************************
 */

#ifndef F_CPU
#define F_CPU           16000000UL             /* clock frequency of target */
#endif // F_CPU

#define BAUD            9600                   /* decimal baud rate */  
#define UBRR_VALUE      (F_CPU/16/BAUD - 1)    /* calculate value for UBRR */


/*
 *******************************************************************************
 *                             FUNCTION PROTOTYPES
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

void usart_init (void);


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

uint8_t usart_receive (void);


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

void usart_transmit (uint8_t data);

#endif //USART0_H
