/*
***********************************************************************************************************************
*                                                   AVR-GENERAL MODULE
*
* File    : SPI.C
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
*
*
* DESCRIPTION:
* Defines standard SPI functions declared in SPI.H that are necessary for initializing the SPI port into master mode,
* and sending/receiving data via SPI on a target device. These are more or less the standard implementations.
*
*
* FUNCTIONS:
*   (1) void    spi_master_init (void);
*   (2) void    spi_master_transmit (uint8_t byte);
*   (3) uint8_t spi_master_read (void);                                                
*
*
*                                                       MIT LICENSE
*
* Copyright (c) 2020 Joshua Fain
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
* documentation files (the "Software"), to deal in the Software without restriction, including without limitation the 
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to 
* permit ersons to whom the Software is furnished to do so, subject to the following conditions: The above copyright 
* notice and this permission notice shall be included in all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***********************************************************************************************************************
*/


#include <stdint.h>
#include <avr/io.h>
#include "../includes/spi.h"



/*
***********************************************************************************************************************
 *                                                       FUNCTIONS
***********************************************************************************************************************
*/

/*
***********************************************************************************************************************
 *                                        INITIALIZE THE SPI PORT INTO MASTER MODE
 * 
 * Description : This function will initialize the SPI port into master mode. This is intended for an ATmega1280 target
 *               and so the PORT assignment for the SPI-specific pins is PORT B. To change this for a different target 
 *               point the relevant MACRO definitions in SPI.H to the correct port.
***********************************************************************************************************************
*/

void 
spi_master_init (void)
{
    // Set MOSI, SCK, and SS of SPI port as outputs. MISO is input.
    DDR_SPI = (1 << DD_MOSI) | (1 << DD_SCK) | (1 << DD_SS0) | (1 << DD_SS1);

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
***********************************************************************************************************************
 *                                        TRANSMIT BYTE IN SPI MASTER MODE
 * 
 * Description  : This function will send a byte via the SPI port operating in master mode to the intended device. It 
 *                does this by loading the byte into SPDR and waiting for the transmission to complete (i.e. the SPIF
 *                flag to be set).
 * 
 * Argument     : byte     Unsigned byte that will be transmitted via the SPI port.
***********************************************************************************************************************
*/

void 
spi_master_transmit (uint8_t byte)
{
    SPDR = byte; //begin data transmission by loading data byte into SPDR
    while ( !(SPSR & (1 << SPIF))); //Wait for transmission complete
}



/*
***********************************************************************************************************************
 *                                        RECEIVE BYTE IN SPI MASTER MODE
 * 
 * Description  : This function is used to get a byte that is received from a device via the SPI in master mode. It 
 *                operates by simply returning the contents of the SPDR register.
 * 
 * Argument     : void
 * 
 * Returns      : Byte received from the device via SPI.
***********************************************************************************************************************
*/

uint8_t 
spi_master_read (void)
{ 
  return SPDR;
}


