/*
*******************************************************************************
*                                AVR-GENERAL MODULE
*
* File    : SPI.C
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
* License : MIT
* Copyright (c) 2020
* 
*
* DESCRIPTION:
* Functions for interacting with the ATMega's SPI port.
*******************************************************************************
*/


#include <stdint.h>
#include <avr/io.h>
#include "../includes/spi.h"





/*
*******************************************************************************
*******************************************************************************
 *                     
 *                                  FUNCTIONS
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
spi_masterInit (void)
{
    // Set MOSI, SCK, and a few SS of SPI port as outputs. MISO is input.
    DDR_SPI =  (1 << DD_MOSI) | (1 << DD_SCK) | (1 << DD_SS0) | (1 << DD_SS1);

    // Make sure SS is high (not asserted) before initializing SPI.
    SPI_PORT = (1 << SS0) | (1 << SS1);
    
    // PRSPI in PPR0 must be 0 to enable SPI. Should be set by default.
    PRR0 &= ~(1 << PRSPI);

    //Enable SPI in master mode and set clock rate ck/64 = 16MHz/64 = 250KHz.
    //SPCR bits: SPIE=0, SPE=1, DORD=0, MSTR=1, CPOL=0, CPHA=0, SPR1=1, SPR0=0
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1);
    SPSR &= ~(1 << SPI2X); //SPI2X = 0. Set to 1 to double clock rate.
}



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
spi_masterTransmit (uint8_t byte)
{
    SPDR = byte; //begin data transmission by loading data byte into SPDR
    while ( !(SPSR & (1 << SPIF))); //Wait for transmission complete
}



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
spi_masterRead (void)
{ 
  return SPDR;
}


