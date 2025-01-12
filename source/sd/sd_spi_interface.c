/*
 * File       : SD_SPI_INTERFACE.C
 * Version    : 1.0
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2025
 * 
 * Implments SD_SPI_INTERFACE.H which provides the functions & macros for
 * interfacing this SD card module with the SPI module of a target device.
 */

#include "sd_spi_interface.h"

/*
 ******************************************************************************
 *                                   FUNCTIONS   
 ******************************************************************************
 */

/*
 * ----------------------------------------------------------------------------
 *                                                            SPI TRANSMIT BYTE
 * 
 * Description : Transmit a byte via SPI to the SD card.
 * 
 * Arguments   : byte   - data byte to be sent to the SD Card via SPI.
 * ----------------------------------------------------------------------------
 */
void sd_TransmitByteSPI(uint8_t byte)
{
  spi_MasterTransmit(byte);                 // sends byte via SPI.
}


/*
 * ----------------------------------------------------------------------------
 *                                                             SPI RECEIVE BYTE
 * 
 * Description : Recieve a byte via SPI from the SD card.
 * 
 * Returns     : byte received from the SD card.
 * ----------------------------------------------------------------------------
 */
uint8_t sd_ReceiveByteSPI(void)
{
  spi_MasterTransmit(DMY_BYTE_SPI);         // send dummy byte to init response
  return spi_MasterReceive();               // return byte received from SD
}


/*
 * ----------------------------------------------------------------------------
 *                                           INITIALIZE SPI PORT IN MASTER MODE 
 * 
 * Description : Initialize the target device's SPI port into Master Mode.
 * ----------------------------------------------------------------------------
 */
void sd_InitMasterModeSPI(void)
{
  SS_DD_OUT;                  // set SPI SS pin as an output pin
  SS_HI_SPI;                  // deassert SD CS pin before enabling SPI
  spi_MasterInit();           // initialize SPI port in master mode
}


/*
 * ----------------------------------------------------------------------------
 *                                                        WAIT SPI CLOCK CYCLES
 * 
 * Description : Wait a specified number of SPI clock cycles. 
 * 
 * Arguments   : spiClkCycles   - desired num of SPI clock cycles to wait. 
 * 
 * Note        : An SPI dummy byte transmit is used to count the number of 
 *               SPI clock cycles to wait. If spiClkCycles is not a multiple of
 *               the SPI data register byte length, then the actual number of 
 *               clock cycles to wait will be the greatest number of SPI
 *               transmits that can complete before going over spiClkCycles.
 * ----------------------------------------------------------------------------
 */
void sd_WaitSpiClkCyclesSPI(uint16_t spiClkCycles)
{
  for (uint8_t wait = 0; wait < NUM_BYTE_TRANS_SPI(spiClkCycles); ++wait)
    sd_TransmitByteSPI(DMY_BYTE_SPI);
}
