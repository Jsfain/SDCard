/*
 * File    : SPI.H
 * Version : 0.0.0.1 
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : MIT
 * Copyright (c) 2020
 * 
 * Interface for interacting with the ATMega's SPI port.
 */

#ifndef SPI_H
#define SPI_H


/*
 ******************************************************************************
 *                                    MACROS   
 ******************************************************************************
 */

// SPI Data Direction Register (DDR).
#define DDR_SPI     DDRB
#define DD_SCK      DDB1
#define DD_MOSI     DDB2
#define DD_MISO     DDB3

// SPI Port Assignment.
#define SPI_PORT    PORTB
#define SCK         PB1 
#define MOSI        PB2
#define MISO        PB3

// Chip Select 0 
#define DD_SS0      DDB0
#define SS0         PB0

// Chip Select 1
#define DD_SS1      DDB4
#define SS1         PB4


/*
 ******************************************************************************
 *                             FUNCTION PROTOTYPES
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

void spi_masterInit (void);


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

uint8_t spi_masterReceive (void);


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

void spi_masterTransmit (uint8_t byte);

#endif  //SPI_H
