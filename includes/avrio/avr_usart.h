/*
 * File       : AVR_USART.H
 * Version    : 1.0 
 * Target     : Default - ATMega1280
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020, 2021
 * 
 * AVR_USART.H provides an interface for accessing and controlling a USART on 
 * the ATMega microcontroller.
 */

#ifndef AVR_USART_H
#define AVR_USART_H

/*
 ******************************************************************************
 *                                  MACROS
 ******************************************************************************
 */

#ifndef F_CPU
#define F_CPU       16000000UL                   // default target clk freq.
#endif //F_CPU

#define BAUD        9600U                        // decimal baud rate
#define UBRR_VALUE  ((F_CPU) / 16 / (BAUD) - 1)  // calculate value for UBRR

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
 * ----------------------------------------------------------------------------
 */
void usart_Init(void);


/*
 * ----------------------------------------------------------------------------
 *                                                           USART RECEIVE BYTE
 *                                         
 * Description : Receives a byte using the USART on the ATmega target device.
 * 
 * Arguments   : void
 * 
 * Returns     : byte received by the USART, i.e. value in UDR0.
 * ----------------------------------------------------------------------------
 */
uint8_t usart_Receive(void);


/*
 * ----------------------------------------------------------------------------
 *                                                          USART TRANSMIT BYTE
 *                                       
 * Description : Sends a byte to another device via the USART.
 * 
 * Arguments   : data   - byte to sent via USART.
 * ----------------------------------------------------------------------------
 */
void usart_Transmit(uint8_t data);

#endif //AVR_USART_H
