/*
 * File       : AVR_SPI.H
 * Version    : 1.0 
 * Target     : ATMega1280
 * Author     : Joshua Fain 2020-2024
 * 
 * Interface for setting and controlling the target AVR device's SPI port.
 * The implementation of the functions are directly based on those provided
 * in the AVR device manual. 
 */

#ifndef AVR_SPI_H
#define AVR_SPI_H

#include <avr/io.h>

/*
 ******************************************************************************
 *                                    MACROS   
 ******************************************************************************
 */

// SPI Data Direction Register (DDR).
#define DDR_SPI     DDRB
#define DD_SS       DDB0
#define DD_SCK      DDB1
#define DD_MOSI     DDB2
#define DD_MISO     DDB3

// SPI Port Assignment.
#define SPI_PORT    PORTB
#define SS          PB0
#define SCK         PB1 
#define MOSI        PB2
#define MISO        PB3

// Common SPI operations. 
#define SS_LO        SPI_PORT  = SPI_PORT & ~(1 << SS)  // set SS pin low
#define SS_HI        SPI_PORT |= 1 << SS                // set SS pin high
#define SS_DD_OUT    DDR_SPI  |= 1 << DD_SS             // set SS pin as output

// Bit length of SPI data register
#define SPI_REG_BIT_LEN      8

/*
 ******************************************************************************
 *                              FUNCTION PROTOTYPES
 ******************************************************************************
 */

/*
 * ----------------------------------------------------------------------------
 *                                         INITIALIZE SPI PORT INTO MASTER MODE
 * 
 * Description : Initialize the target's SPI port into master mode.
 * 
 * Note        : If an application is using a different pin for Chip Select
 *               other than the SS pin of the AVR'S SPI port, then that
 *               application must define it and set the data direction to
 *               output. It should also ensure the pin is de-asserted prior to
 *               calling this function so the device's SPI port will not be
 *               active at the moment it is enabled on the AVR.
 * ----------------------------------------------------------------------------
 */
void spi_MasterInit(void);

/*
 * ----------------------------------------------------------------------------
 *                                                             SPI RECEIVE BYTE
 *                                       
 * Description : Gets byte sent to the SPDR from an SPI connected device.  
 * 
 * Returns     : byte received by the SPI port's data register (SPDR).
 * ----------------------------------------------------------------------------
 */
uint8_t spi_MasterReceive(void);

/*
 * ----------------------------------------------------------------------------
 *                                                            SPI TRANSMIT BYTE 
 * 
 * Description : Sends a byte via the SPI port operating in master mode.
 * 
 * Arguments   : byte - data byte to be sent via SPI.
 * ----------------------------------------------------------------------------
 */
void spi_MasterTransmit(uint8_t byte);

#endif  //AVR_SPI_H
