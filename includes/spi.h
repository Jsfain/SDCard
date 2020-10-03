/******************************************************************************
 * Copyright (c) 2020 Joshua Fain
 * 
 * 
 * SPI.H
 * 
 * 
 * DESCRIPTION
 * Defines SPI port/pins and declares SPI specific functions.
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
 * ***************************************************************************/


#ifndef SPI_H
#define SPI_H


/******************************************************************************
 * Definitions: SPI port/pins data direction definitions.
 * Description: Define the data direction variables of the SPI port/pins for 
 *              the target device.
******************************************************************************/
#define DDR_SPI     DDRB
#define DD_SS       DDB0
#define DD_SCK      DDB1
#define DD_MOSI     DDB2
#define DD_MISO     DDB3



/******************************************************************************
 * Definitions: SPI port pins.  
 * Description: Define the SPI port/pins for the targe device.
******************************************************************************/
#define SPI_PORT    PORTB
#define SS          PB0
#define SCK         PB1
#define MOSI        PB2
#define MISO        PB3



/******************************************************************************
 * Functions: Declaration of standard SPI initialization, transmit, and 
 *            receive functions. See SPI.C for a more detailed descriptions.
******************************************************************************/

// Initialialized the SPI port in master mode
void SPI_MasterInit(void);

// Sends byte via SPI to device
void SPI_MasterTransmit(uint8_t cData);

// Receives byte via SPI from device
uint8_t SPI_MasterRead();

#endif  //SPI_H