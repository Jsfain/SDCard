/*
 * File       : SPI.H
 * Version    : 1.0 
 * Target     : ATMega1280
 * Compiler   : AVR-GCC 9.3.0
 * Downloader : AVRDUDE 6.3
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020, 2021
 * 
 * Interface for interacting with the ATMega's SPI port.
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
 * Description : Initialize the SPI port into master mode.
 * 
 * Arguments   : void
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */
void spi_MasterInit(void);

/*
 * ----------------------------------------------------------------------------
 *                                                             SPI RECEIVE BYTE
 *                                       
 * Description : Gets byte sent to the SPDR by an SPI connected device.  
 * 
 * Arguments   : void
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
 * Arguments   : byte     data byte to be sent via SPI.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */
void spi_MasterTransmit(uint8_t byte);

#endif  //SPI_H
