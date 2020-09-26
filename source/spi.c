/*************************************************************************
 * Author: Joshua Fain
 * Date:   6/23/2020
 * 
 * 
 * File:     SPI.C 
 * 
 * Requires: SPI.H     - needed for sending commands to, and receiving
 *                       responses from, SD Card.
 *           STDINT.H  - defines data types used here.
 *           AVR/IO.H  - needed for I/O related AVR variables.
 * 
 * 
 * Target: ATmega 1280
 * 
 * Description: 
 * Defines standard SPI functions declared in SPI.H that are necessary 
 * for initializing the SPI port into master mode, and sending/receiving
 * data via SPI on the target device. These are mostly the same as those
 * found in the ATmega datasheet.
 * 
 * Functions:
 * 1) void SPI_MasterInit(void)
 * 2) void SPI_MasterTransmit(char cData)
 * 3) uint8_t SPI_MasterRead()
 * **********************************************************************/


#include <stdint.h>
#include <avr/io.h>
#include "../includes/spi.h"


/******************************************************************************
 * Function:    SPI_MasterInit(void)
 * Description: Initializes SPI port into master mode.
 * Argument(s): VOID
 * Returns:     VOID
******************************************************************************/
void SPI_MasterInit(void)
{
    // Set MOSI, SCK, and SS of SPI port as outputs. MISO is input.
    DDR_SPI = (1<<DD_MOSI)|(1<<DD_SCK)|(1<<DD_SS);

    //ensure SS is high (not asserted) before initializing SPI.
    SPI_PORT = 0x01;
    
    //PRSPI in PPR0 must be 0 to enable SPI. Should be by default.
    PRR0 &= ~(1<<PRSPI);

    //Enable SPI in master mode and set clock rate ck/64 = 16MHz/64 = 250KHz.
    //SPCR bits: SPIE=0, SPE=1, DORD=0, MSTR=1, CPOL=0, CPHA=0, SPR1=1, SPR0=0
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1);
    SPSR &= ~(1<<SPI2X); //SPI2X = 0. Set to 1 to double clock rate.
}



/******************************************************************************
 * Function:    SPI_MasterTransmit(char cData)
 * Description: Sends data byte via SPI by loading data into SPDR
 * Argument(s): 8-bit data byte to send via SPI
 * Returns:     VOID   
******************************************************************************/
void SPI_MasterTransmit(uint8_t cData)
{
    SPDR = cData; //begin data transmission by loading data byte into SPDR
    while(!(SPSR & (1<<SPIF))); //Wait for transmission complete
}


/******************************************************************************
 * Function:    SPI_MasterRead(void)
 * Description: gets response byte from SPI device. 
 * Argument(s): VOID
 * Returns:     8-bit data byte returned from the SPI device into the SPDR.
******************************************************************************/
uint8_t SPI_MasterRead(void)
{ return SPDR; }


