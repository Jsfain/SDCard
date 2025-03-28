/*
 * File       : SD_SPI_INTERFACE.H
 * Version    : 1.0
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2025
 * 
 * SD_SPI_INTERFACE required for interfacing this SD card module with the SPI
 * module of the provided target device. A target-specific SPI module must be 
 * included here capable of initializing the SPI unit into master mode as well
 * as handling all other SPI-specific functionality - see the definitions here
 * and the accompanying source file for details. Changing the target device 
 * should only require updating definitions here and the source file if needed. 
 * 
 * DO NOT change any function or macro names here and ensure any definition 
 * changes preserve the defined behavior expected by the SD card module.
 * 
 * NOTE: All functions & macros defined here end in 'SPI' to distinguish them
 *       from the SD-specific functions in the rest of the SD card module.
 */
#ifndef SD_SPI_INTERFACE_H
#define SD_SPI_INTERFACE_H

#include "avr_spi.h"          // include target device's SPI module

/*
 ******************************************************************************
 *                                   MACROS
 ******************************************************************************
 */

// 
// These macros will be used to define CS_ASSERT and CS_DEASSERT in SD_SPI_BASE
// to control the SD card's Chip Select (CS) pin to enable/disable SPI comm on
// the card. SS_LO and SS_HI should be defined in the included SPI module and 
// must pull the SPI port's SS pin low and high, respectively.
//
#define SS_LO_SPI    SS_LO               // pulls SPI SS pin low
#define SS_HI_SPI    SS_HI               // pulls SPI SS pin high

//
// Dummy byte. The SPI clock cycle begins once a byte is loaded into the SPI 
// data register to transmit or receive a data byte. During a transmit, the
// byte loaded into the data register is data to be sent to the SD card. During 
// a receive, the value of the byte doesn't matter and this dummy byte is used
// instead to initiate the SPI clock cycle by loading into the data register.
// This should be standard for all SPI units, but should be verified for each
// target device used.
//
#define DMY_BYTE_SPI 0xFF


//
// Bit length of the SPI Data Register. The value should be 8.
//
#define DATA_REG_LEN_SPI    SPI_REG_BIT_LEN


// 
// Returns the number of complete SPI byte transmits that will occur before the
// the number of specified SPI clock cycles (clkCycles) is reached.
//
#define NUM_BYTE_TRANS_SPI(clkCycles) clkCycles / DATA_REG_LEN_SPI


/*
 ******************************************************************************
 *                                 FUNCTIONS
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
void sd_TransmitByteSPI(uint8_t byte);

/*
 * ----------------------------------------------------------------------------
 *                                                             SPI RECEIVE BYTE
 * 
 * Description : Recieve a byte via SPI from the SD card.
 * 
 * Returns     : byte received from the SD card.
 * ----------------------------------------------------------------------------
 */
uint8_t sd_ReceiveByteSPI(void);

/*
 * ----------------------------------------------------------------------------
 *                                           INITIALIZE SPI PORT IN MASTER MODE 
 * 
 * Description : Initialize the target device's SPI port into Master Mode.
 * ----------------------------------------------------------------------------
 */
void sd_InitMasterModeSPI(void);


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
 *               the SPI data register bit length, then the actual number of 
 *               clock cycles to wait will be the greatest number of SPI
 *               transmits that can complete before going over spiClkCycles.
 * ----------------------------------------------------------------------------
 */
void sd_WaitSpiClkCyclesSPI(uint16_t clkCycles);


#endif //SD_SPI_INTERFACE_H
