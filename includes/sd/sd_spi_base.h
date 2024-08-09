/*
 * File       : SD_SPI_BASE.H
 * Version    : 1.0
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020 - 2024
 * 
 * SD_SPI_BASE provides functions and macros required for the basic interaction 
 * with an SD card in SPI mode.
 *  
 * NOTE:
 * An SPI interface module must be included to handle the SPI-specific
 * operations on the target device. The following is a list of the SPI macros
 * and functions defined in the included SPI module that are required for this 
 * SD module to function.
 * 
 * SPI Macros   :  SS_HI           - sets the master SPI SS pin to HI(1). Used 
 *                                   to define CS_DEASSERT in this file.
 *                 SS_LO           - sets the master SPI SS pin to LO(0). Used 
 *                                   to define CS_ASSERT in this file.
 *                 SS_DD_OUT       - sets the data direction of the SPI SS pin
 *                                   so it operates as an output pin. Used in 
 *                                   pvt_initSPI in sd_spi_base.c.
 *                 SPI_REG_BIT_LEN - the bit length of the SPI data register 
 *                                   to calculate number of SPI clock cycles in
 *                                   sd_WaitSPI in sd_spi_base.c.
 * 
 * The below SPI-specific fuctions are called from functions in sd_spi_base.c
 * 
 * SPI Functions : spi_MasterInit     - initializes target's SPI port into
 *                                      master mode. Called in pvt_initSPI.
 *                 spi_MasterTransmit - transmits single byte via SPI. Called 
 *                                      in sd_SendByteSPI
 *                 spi_MasterReceive  - receives single byte via SPI. Called 
 *                                      in sd_ReceiveByteSPI
 */

#ifndef SD_SPI_BASE_H
#define SD_SPI_BASE_H

#include "avr_spi.h"          // SPI module
#include "sd_spi_car.h"       // defs for SD Commands, Arguments, Responses

/*
 ******************************************************************************
 *                                   MACROS
 ******************************************************************************
 */

// 
// CS_ASSERT and CS_DEASSERT control the SD card's Chip Select (CS) pin to 
// enable and disable SPI communication to the card.
// 
#define CS_ASSERT       SS_LO               // enables card by setting CS low
#define CS_DEASSERT     SS_HI               // disables card by setting CS high

// Used for Send Command
#define TX_CMD_BITS     0x40                // transmit bits (msb = 01)
#define STOP_BIT        0x01                // final bit sent in a cmd/arg

// Max number of attempts to check for valid command response from SD card.
#define MAX_ATTEMPTS    0xFE  

// Dummy token. Used when waiting on, or initiating a response via SPI.
#define DMY_TKN         0xFF

// Card Versions
#define VERSION_1       1
#define VERSION_2       2

// Card Types - standard or high capacity
#define SDHC            1                   // High Cap - block addressable
#define SDSC            0                   // Std. Cap - byte addressable


/* 
 * ----------------------------------------------------------------------------
 *                                                        HOST CAPACITY SUPPORT
 * 
 * Description : Setting determines the card type the host will support.
 *        
 * Notes       : Set to either SDHC or SDSC. Recommend setting to SDHC, which 
 *               supports SDSC by default. This setting is used to inform the 
 *               card the version the host is capable of supporting.
 * ----------------------------------------------------------------------------
 */
#define HOST_CAPACITY_SUPPORT  SDHC


/* 
 * ----------------------------------------------------------------------------
 *                                                                 BLOCK LENGTH
 *
 * Description : The SD card block length (in bytes) assumed by the host. 
 *       
 * Warning     : This should always be set to 512. If not, the application will 
 *               produce unexpected results and/or fail.
 * ----------------------------------------------------------------------------
 */
#define BLOCK_LEN       512


/* 
 * ----------------------------------------------------------------------------
 *                                                   INITIALIZATION ERROR FLAGS
 * 
 * Description : Error flags that may be returned by sd_InitModeSPI if an issue 
 *               is encountered during initialization.
 *        
 * Notes       : 1) These flags are specific to this module's init function,
 *                  they are not part of the SD std.
 *               2) The most recent R1 response during init is returned in the
 *                  LSB of the init routine's returned value. Hence, these init 
 *                  error flags begin at the second byte, the lower byte is 
 *                  reserved for the R1 response. See SD_SPI_CAR.H for R1 list.
 * ----------------------------------------------------------------------------
 */
#define FAILED_GO_IDLE_STATE    0x00100     // CMD0 error
#define FAILED_SEND_IF_COND     0x00200     // CMD8 error
#define UNSUPPORTED_CARD_TYPE   0x00400     // CMD8 and CMD58 error
#define FAILED_CRC_ON_OFF       0x00800     // CMD59 error
#define FAILED_APP_CMD          0x01000     // CMD55 error
#define FAILED_SD_SEND_OP_COND  0x02000     // ACMD41 error
#define OUT_OF_IDLE_TIMEOUT     0x04000     // ACMD41 error
#define FAILED_READ_OCR         0x08000     // CMD58 error
#define POWER_UP_NOT_COMPLETE   0x10000     // CMD58 error


/*
 * ----------------------------------------------------------------------------
 *                                OPERATION CONDITIONS REGISTER (OCR) BIT MASKS
 * 
 * Description : Bit masks used to check settings of the SD card's OCR.
 * ----------------------------------------------------------------------------
 */
#define POWER_UP_BIT_MASK     0x80
#define CCS_BIT_MASK          0x40          // Card Capacity Support
#define UHSII_BIT_MASK        0x20          // UHS-II Card Status
#define CO2T_BIT_MASK         0x10          // Over 2TB support status
#define S18A_BIT_MASK         0x08          // switching to 1.8V accepted

// Volt Range Accepted by card: 2.7 - 3.6V. Only this range has been tested.
#define VRA_OCR_MASK          0xFF80


/*
 ******************************************************************************
 *                                   STRUCTS
 ******************************************************************************
 */

/* 
 * ----------------------------------------------------------------------------
 *                                                        CARD TYPE and VERSION
 * 
 * Members  : 1) version - SD Card's version. Either 1 or 2.
 *            2) type    - SD Card's type. Either SDSC or SDHC.
 * 
 * Notes    : 1) The members of this struct should only be set by passing a 
 *               pointer of an instance to the init routine, sd_InitModeSPI.
 *            2) The value of 'type' is necessary for determining how a card's 
 *               blocks are addressed.
 * 
 * Warnings : Only version 2 cards have been tested.
 * ----------------------------------------------------------------------------
 */
typedef struct CardTypeVersion{
    uint8_t version;
    uint8_t type;
} CTV;

/*
 ******************************************************************************
 *                           FUNCTION PROTOTYPES
 ******************************************************************************
 */

/*
 * ----------------------------------------------------------------------------
 *                                          WAIT MAX NUMBER OF SPI CLOCK CYCLES
 * 
 * Description : Used to wait a specified max number of SPI clock cycles by
 *               repeatedly sending DMY_TKN via SPI.
 * 
 * Arguments   : clkCycles   - max num of SPI clock cycles to wait. 
 * 
 * Note        : 1)  SPI_REG_BIT_LEN is included via SPI module and is just the
 *                   bit length of the SPI data register.
 *               2)  If clkCycles is not a multiple of the SPI bit length, then 
 *                   the actual number of clock cycles to wait will be the 
 *                   greatest multiple of SPI bit length below the value of
 *                   clkCycles.
 * ----------------------------------------------------------------------------
 */
void sd_WaitSPI(uint16_t clkCycles);

/*
 * ----------------------------------------------------------------------------
 *                                                       SD CARD INITIALIZATION
 *
 * Description : Implements the SD Card SPI mode initialization routine and 
 *               sets the members of the CTV (Card Type and Version) struct
 *               instance. 
 *
 * Arguments   : ctv - ptr to CTV instance whose members are set during init.
 * 
 * Returns     : Initialization Response. This includes any Initialization
 *               Error Flags set in bits 8 to 16 and the most recent R1 
 *               response in the lowest byte.
 * 
 * Warning     : An instance of CTV should ONLY be set by this function.
 * ----------------------------------------------------------------------------
 */
uint32_t sd_InitModeSPI(CTV *ctv);

/*
 * ----------------------------------------------------------------------------
 *                                                                    SEND BYTE
 * 
 * Description : Sends a single 8-bit byte to the SD card via the SPI port.
 * 
 * Arguments   : byte   - byte to be sent to the SD Card via SPI.
 * 
 * Notes       : 1) Call this function as many times as necessary to send the 
 *                  complete data packet, token, command, etc...
 *               2) This function calls spi_MasterTransmit. This, or similarly 
 *                  operating function must be included to perform the SPI 
 *                  transmit byte operation via SPI port in master mode.
 * ----------------------------------------------------------------------------
 */
void sd_SendByteSPI(uint8_t byte);

/*
 * ----------------------------------------------------------------------------
 *                                                                 RECEIVE BYTE
 * 
 * Description : Receive a single 8-bit byte from the SD card via the SPI port.
 * 
 * Returns     : byte received from the SD card.
 * 
 * Notes       : 1) Call this function as many times as necessary to retrieve 
 *                  the complete data packet, token, error response, etc...
 *               2) This function calls spi_MasterReceive. This, or a similarly 
 *                  operating function must be included to perform the SPI 
 *                  receive byte operation via SPI port in master mode.
 * ----------------------------------------------------------------------------
 */
uint8_t sd_ReceiveByteSPI(void);

/*
 * ----------------------------------------------------------------------------
 *                                                                 SEND COMMAND
 * 
 * Description : Send a command and argument to the SD Card. See sd_spi_car.h 
 *               for the list of command and argument macros.
 * 
 * Arguments   : cmd   - SD Card command.
 *               arg   - 32-bit argument to be sent with the SD command.
 * ----------------------------------------------------------------------------
 */
void sd_SendCommand(uint8_t cmd, uint32_t arg);

/*
 * ----------------------------------------------------------------------------
 *                                                              GET R1 RESPONSE
 * 
 * Description : Retrieves the R1 response from the SD card after it has been 
 *               sent a command.
 * 
 * Returns     : R1 response flag(s). See sd_spi_car.h.
 * 
 * Notes       : 1) always call immediately after calling sd_SendCommand.
 *               2) if R1_TIMEOUT is returned, then the SD Card did not return
 *                  a response within the specified number of attempts.
 * ----------------------------------------------------------------------------
 */
uint8_t sd_GetR1(void);

#endif //SD_SPI_BASE_H
