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
* Defines SPI port/pins and declares SPI specific functions.
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