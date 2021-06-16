/*
 * File       : AVR_SPI.H
 * Version    : 1.0 
 * Target     : Default - ATMega1280
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020, 2021
 * 
 * AVR_SPI.H provides the interface or accessing and controlling the ATMega 
 * microcontroller's SPI port.
 */

#ifndef SPI_H
#define SPI_H

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
 * Description : Initialize the AVR's SPI port into master mode.
 * 
 * Note        : If an application is using a different pin for Chip Select
 *               other than the SS pin of the AVR'S SPI port, then that
 *               application must define it and set the data direction to
 *               output. It should also ensure that the pin is deasserted prior
 *               to calling this function. This is to ensure that the device's
 *               SPI port is not active at the moment it is enabled on the AVR.
 * ----------------------------------------------------------------------------
 */
void spi_MasterInit(void);

/*
 * ----------------------------------------------------------------------------
 *                                                             SPI RECEIVE BYTE
 *                                       
 * Description : Gets byte sent to the SPDR by an SPI connected device.  
 *
 * Returns     : byte received by the SPI port.
 * ----------------------------------------------------------------------------
 */
uint8_t spi_MasterReceive(void);

/*
 * ----------------------------------------------------------------------------
 *                                                            SPI TRANSMIT BYTE 
 * 
 * Description : Sends a byte via the SPI port operating in master mode.
 * 
 * Arguments   : byte   - data byte to be sent via SPI.
 * ----------------------------------------------------------------------------
 */
void spi_MasterTransmit(uint8_t byte);

#endif  //SPI_H
