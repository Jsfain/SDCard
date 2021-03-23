/*
 * File    : SPI.C
 * Version : 1.0 
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : GNU GPLv3
 * Copyright (c) 2020, 2021
 * 
 * Implementation of SPI.H
 */

#include <stdint.h>
#include <avr/io.h>
#include "spi.h"

/*
 ******************************************************************************
 *                                  FUNCTIONS
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
void spi_MasterInit(void)
{
  // Set MOSI, SCK, SS pins of SPI port as outputs.
  DDR_SPI =  1 << DD_MOSI | 1 << DD_SCK | 1 << DD_SS0 | 1 << DD_SS1;

  // Make sure SS pins are high (not asserted) before initializing SPI.
  SPI_PORT = 1 << SS0 | 1 << SS1;
  
  // PRSPI in PPR0 must be 0 to enable SPI. Should be 0 by default.
  PRR0 &= ~(1 << PRSPI);

  //Enable SPI in master mode. Set clock rate: ck/64 = 16MHz/64 = 250KHz.
  //SPCR: SPIE=0, SPE=1, DORD=0, MSTR=1, CPOL=0, CPHA=0, SPR1=1, SPR0=0
  SPCR = 1 << SPE | 1 << MSTR | 1 << SPR1;

  //SPI2X = 0. Set to 1 to double clock rate.
  SPSR &= ~(1 << SPI2X);
}

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
uint8_t spi_MasterReceive(void)
{ 
  return SPDR;
}

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
void spi_MasterTransmit(uint8_t byte)
{
  // load byte into SPDR to transmit data.
  SPDR = byte;

  // wait for data transmission to complete.
  while ( !(SPSR & 1 << SPIF))
    ;
}
