/*
 * File       : AVR_SPI.C
 * Version    : 1.0
 * Target     : ATMega1280
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020 - 2024
 * 
 * Description: Implements AVR_SPI.H for setting and controlling the target 
 *              AVR device's SPI port.
 */

#include <avr/io.h>
#include "avr_spi.h"

/*
 ******************************************************************************
 *                                  FUNCTIONS
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
 *               output. It should also ensure the pin is de-asserted prior to
 *               calling this function so the device's SPI port will not be
 *               active at the moment it is enabled on the AVR.
 * ----------------------------------------------------------------------------
 */
void spi_MasterInit(void)
{
  //
  // Set MOSI and SCK pins of SPI port as outputs. MISO is input. The SS pin
  // must also be set to output before enabling master mode, regardless of
  // whether it is used as a device's chip select.
  //
  DDR_SPI |= 1 << DD_MOSI | 1 << DD_SCK | 1 << DD_SS;

  // Set SS pin high before enabling SPI port. Used as Chip Select.
  SPI_PORT |= 1 << SS;
  
  // PRSPI in PPR0 must be 0 to enable SPI. Should be 0 by default.
  PRR0 &= ~(1 << PRSPI);

  //Enable SPI in master mode. Set clock rate: ck/64 = 16MHz/64 = 250kHz.
  //SPCR: SPIE=0, SPE=1, DORD=0, MSTR=1, CPOL=0, CPHA=0, SPR1=1, SPR0=0
  SPCR = 1 << SPE | 1 << MSTR | 1 << SPR1;

  //SPI2X = 0. Set to 1 to double clock rate.
  SPSR &= ~(1 << SPI2X);
}

/*
 * ----------------------------------------------------------------------------
 *                                                             SPI RECEIVE BYTE
 *                                       
 * Description : Gets byte sent to the SPDR from an SPI connected device.  
 * 
 * Returns     : byte received by the SPI port's data register (SPDR).
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
 * Arguments   : byte - data byte to be sent via SPI.
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
