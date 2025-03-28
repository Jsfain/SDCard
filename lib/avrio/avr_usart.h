/*
 * File       : AVR_USART.H
 * Version    : 1.0 
 * Target     : ATMega1280
 * Author     : Joshua Fain 2020-2023
 * 
 * Interface for accessing and controlling the USART on the target AVR device.
 * The implementation of the functions are directly based on those provided in
 * the AVR device manual. 
 */

#ifndef AVR_USART_H
#define AVR_USART_H

/*
 ******************************************************************************
 *                                  MACROS
 ******************************************************************************
 */

#ifndef F_CPU
#define F_CPU       16000000UL                       // AVR target's clk freq.
#endif //F_CPU

#define BAUD        9600U                            // decimal baud rate
#define ASYNC_MODE  16                               // Asynch normal mode
#define UBRR_VALUE  (F_CPU / ASYNC_MODE / BAUD - 1)  // calculate UBRR. 

/*
 *******************************************************************************
 *                             FUNCTION PROTOTYPES
 ******************************************************************************
 */

/*
 * ----------------------------------------------------------------------------
 *                                                             INITIALIZE USART
 *                                        
 * Description : Initialize USART on the AVR target device.
 * ----------------------------------------------------------------------------
 */
void usart_Init(void);


/*
 * ----------------------------------------------------------------------------
 *                                                                USART RECEIVE
 *                                         
 * Description : Receive a character via USART on the AVR target device.
 * 
 * Returns     : character received by the USART, i.e. value in UDR0.
 * ----------------------------------------------------------------------------
 */
uint8_t usart_Receive(void);


/*
 * ----------------------------------------------------------------------------
 *                                                               USART TRANSMIT
 *                                       
 * Description : Sends a character via USART.
 * 
 * Arguments   : data - data to send via USART.
 * ----------------------------------------------------------------------------
 */
void usart_Transmit(uint8_t data);


/*
 * ----------------------------------------------------------------------------
 *                                                               USART TRANSMIT
 *                                       
 * Description : Returns the immediate contents of the eturn
 * 
 * Arguments   : data - data to send via USART.
 * ----------------------------------------------------------------------------
 */
void usart_ReadDataRegister(uint8_t data);

/*
 * ----------------------------------------------------------------------------
 *                                                   USART RECEIVE BUFFER FLUSH
 *                                         
 * Description : Flush the USART receive buffer.
 * ----------------------------------------------------------------------------
 */
void usart_Flush(void);

#endif //AVR_USART_H
