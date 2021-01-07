/*
*******************************************************************************
*                                AVR-GENERAL MODULE
*
* File    : SPI.H
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
* License : MIT
* Copyright (c) 2020
* 
*
* DESCRIPTION:
* Interface for interacting with the ATMega's SPI port.
*******************************************************************************
*/


#ifndef SPI_H
#define SPI_H





/*
*******************************************************************************
*******************************************************************************
 *                     
 *                                 MACROS   
 *  
*******************************************************************************
*******************************************************************************
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

// Chip Select 2
#define DD_SS2      DDB5
#define SS2         PB5


/*
*******************************************************************************
*******************************************************************************
 *                     
 *                           FUNCTION DECLARATIONS 
 *  
*******************************************************************************
*******************************************************************************
*/

/*
-------------------------------------------------------------------------------
|                  INITIALIZE THE SPI PORT INTO MASTER MODE 
|                                        
| Description : Initialize the SPI port into master mode.
-------------------------------------------------------------------------------
*/

void 
spi_masterInit (void);



/*
-------------------------------------------------------------------------------
|                       TRANSMIT BYTE IN SPI MASTER MODE
|                                        
| Description : Sends a byte via the SPI port operating in master mode.
|
| Arguments   : byte   - byte to be sent via SPI.
-------------------------------------------------------------------------------
*/

void 
spi_masterTransmit (uint8_t byte);



/*
-------------------------------------------------------------------------------
|                       RECEIVE BYTE IN SPI MASTER MODE
|                                        
| Description : Gets the byte in the SPDR that was sent to the microcontroller 
|               via SPI from a connected device.
|
| Returns     : byte received via SPI.
-------------------------------------------------------------------------------
*/

uint8_t 
spi_masterRead (void);



#endif  //SPI_H
