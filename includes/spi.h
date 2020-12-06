/*
***********************************************************************************************************************
*                                                   AVR-GENERAL MODULE
*
* File    : SPI.H
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
*
*
* DESCRIPTION:
* Defines SPI port/pins and declares SPI specific functions defined in SPI.C that are necessary for initializing the 
* SPI port into master mode, and sending/receiving data via SPI on a target device. These are more or less the 
* standard implementations.
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


#ifndef SPI_H
#define SPI_H


/*
***********************************************************************************************************************
 *                                                       MACROS
***********************************************************************************************************************
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
spi_master_init (void);



/*
***********************************************************************************************************************
 *                                        TRANSMIT BYTE IN SPI MASTER MODE
 * 
 * Description  : This function will send a byte via the SPI port operating in master mode to the intended device. It 
 *                does this by loading the byte into SPDR and waiting for the transmission to complete (i.e. the SPIF
 *                flag to be set).
 * 
 * Argument     : byte     unsigned byte that will be transmitted via the SPI port.
***********************************************************************************************************************
*/

void 
spi_master_transmit (uint8_t byte);



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
spi_master_read (void);



#endif  //SPI_H
