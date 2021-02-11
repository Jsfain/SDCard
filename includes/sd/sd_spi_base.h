/*
 * File    : SD_SPI_BASE.H
 * Version : 0.0.0.1 
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : MIT
 * Copyright (c) 2020-2021
 * 
 * Provides several functions and macros for basic interaction between an SD
 * SD Card and an AVR microcontroller when operating in SPI mode.
 */

#ifndef SD_SPI_H
#define SD_SPI_H

#include "sd_spi_cmds.h"

/*
 ******************************************************************************
 *                                   MACROS
 ******************************************************************************
 */

/* 
 * ----------------------------------------------------------------------------
 *                                                                TIMEOUT LIMIT
 *
 * Description : This value determines the limit on number of attempts to get
 *               valid values. The value can be any value < 0xFF. This is 
 *               typically applied to loops polling for an SD card 
 *               response/data. This does not represent a "time" value but
 *               how many times an attempt to return a valid value will be made
 *               before returning from a function with an error. 
 * 
 * Notes       : Some functions can have timeouts >= 0xFF and these are
 *               implemented as multiples of and/or additions to this value.
 * ----------------------------------------------------------------------------
 */
#define TIMEOUT_LIMIT           0xFE

/* 
 * ----------------------------------------------------------------------------
 *                                                                 BLOCK LENGTH
 *
 * Description : The SD card block length (in bytes) assumed by the host. 
 *       
 * Warning     : This should always be set to 512. If not the module will 
 *               produce unexpected results and/or fail.
 * ----------------------------------------------------------------------------
 */
#define BLOCK_LEN               512

/* 
 * ----------------------------------------------------------------------------
 *                                                                  CHIP SELECT
 * 
 * Notes       : 1) Assert by setting CS low. De-Assert by setting CS high.
 *               
 *               2) SSO, defined in SPI.H, is used here as the CS pin. 
 *  
 *               3) SPI.H also defines a second SS pin (SS1) for connecting 
 *                  multiple devices to the SPI port of the AVR. Therefore, 
 *                  this MACRO must also ensure the SS1 pin is de-asserted 
 *                  (set high) when SS0 is asserted (set low).
 * ----------------------------------------------------------------------------
 */
#define CS_SD_LOW    SPI_PORT = ((SPI_PORT & ~(1 << SS0)) | (1 << SS1));
#define CS_SD_HIGH   SPI_PORT |=  (1 << SS0);

/* 
 * ----------------------------------------------------------------------------
 *                                                            R1 RESPONSE FLAGS
 * 
 * Description : Flags returned by sd_getR1().
 * 
 * Notes       : 1) With the exception of R1_TIMEOUT, these flags correspond to
 *                  the first byte returned by the SD card in response to any 
 *                  command.
 *      
 *               2) R1_TIMEOUT will be set in the sd_getR1() return value if 
 *                  the SD card does not send an R1 response after set amount
 *                  of time.
 * ----------------------------------------------------------------------------
 */
#define OUT_OF_IDLE             0x00
#define IN_IDLE_STATE           0x01
#define ERASE_RESET             0x02
#define ILLEGAL_COMMAND         0x04
#define COM_CRC_ERROR           0x08
#define ERASE_SEQUENCE_ERROR    0x10
#define ADDRESS_ERROR           0x20
#define PARAMETER_ERROR         0x40
#define R1_TIMEOUT              0x80

/* 
 * ----------------------------------------------------------------------------
 *                                                   INITIALIZATION ERROR FLAGS
 * 
 * Description : Flags returned in bits 8 to 19 by sd_spiModeInit().
 *        
 * Notes       : The lowest byte returned by sd_spiModeInit() is the most 
 *               recent R1 Response.
 * ----------------------------------------------------------------------------
 */
#define FAILED_GO_IDLE_STATE    0x00100                    // CMD0
#define FAILED_SEND_IF_COND     0x00200                    // CMD8
#define UNSUPPORTED_CARD_TYPE   0x00400
#define FAILED_CRC_ON_OFF       0x00800                    // CMD59
#define FAILED_APP_CMD          0x01000                    // CMD55
#define FAILED_SD_SEND_OP_COND  0x02000                    // ACMD41
#define OUT_OF_IDLE_TIMEOUT     0x04000
#define FAILED_READ_OCR         0x08000                    // CMD58
#define POWER_UP_NOT_COMPLETE   0x10000

/* 
 * ----------------------------------------------------------------------------
 *                                                      CARD TYPES and VERSIONS
 * 
 * Notes       : 1) High/extended capacity (SDHC) cards are block addressable.
 *               
 *               2) Standard capacity (SDSC) cards are byte addressable. 
 * ----------------------------------------------------------------------------
 */
#define SDHC           1 
#define SDSC           0

#define VERSION_1      1
#define VERSION_2      2

/* 
 * ----------------------------------------------------------------------------
 *                                                        HOST CAPACITY SUPPORT
 * 
 * Description : Specifies the card type(s) that the host will support.
 *        
 * Notes       : 1) Set to either SDHC or SDSC.
 * 
 *               2) Advise setting to SDHC, which will also support SDSC.
 * 
 *               3) Setting to SDSC will cause the host to only support SDSC 
 *                  cards. 
 * ----------------------------------------------------------------------------
 */
#define HOST_CAPACITY_SUPPORT  SDHC

/*
 ******************************************************************************
 *                                   STRUCTS
 ******************************************************************************
 */

/* 
 * ----------------------------------------------------------------------------
 *                                                        CARD TYPE and VERSION
 * 
 * Members : 1) version - SD Card's version. Either 1 or 2.
 *  
 *           2) type    - SD Card's type. Either SDSC or SDHC.
 * 
 * Notes   : 1) The members of this struct should only be set by the 
 *              initialization routine sd_spiModeInit(*ctv).
 *         
 *           2) The value of 'type' can be used to determine how the card is 
 *              addressed by any command accessing data blocks.
 * 
 * Warnings : Only version 2 cards have been tested.
 * ----------------------------------------------------------------------------
 */
typedef struct {
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
 *                                                       SD CARD INITIALIZATION
 *
 * Description : Implements the SD Card SPI mode initialization routine and 
 *               sets the members of the CTV instance. 
 *
 * Arguments   : ctv     ptr to a CTV instance whose members will be set here.
 * 
 * Returns     : Initialization Error Response. This includes initialization 
 *               error flag in bits 8 to 19 and the most recent R1 response in
 *               the lowest byte.
 * -----------------------------------------------------------------------------
 */
uint32_t sd_InitModeSPI(CTV* ctv);

/*
 * ----------------------------------------------------------------------------
 *                                                                    SEND BYTE
 * 
 * Description : Sends a single byte to the SD card via the SPI port.
 * 
 * Arguments   : byte     8-bits that will be sent to the SD Card via SPI.
 * 
 * Returns     : void
 * 
 * Notes       : 1) Call as many times as required to send the complete data 
 *                  packet, token, command, etc...
 * 
 *               2) This, and sd_receiveByteSPI(), are the SPI interfacing 
 *                  functions.
 * ----------------------------------------------------------------------------
 */
void sd_sendByteSPI(uint8_t byte);

/*
 * ----------------------------------------------------------------------------
 *                                                                 RECEIVE BYTE
 * 
 * Description : Receives/returns single byte from the SD card via SPI port.
 * 
 * Arguments   : void
 * 
 * Returns     : 8-bit byte received from the SD card.
 * 
 * Notes       : 1) Call as many times as necessary to get the complete data
 *                  packet, token, error response, etc... from the SD card.
 * 
 *               2) This, and sd_sendByteSPI(), are the SPI interfacing 
 *                  functions.
 * ----------------------------------------------------------------------------
 */
uint8_t sd_receiveByteSPI(void);

/*
 * ----------------------------------------------------------------------------
 *                                                                 SEND COMMAND
 * 
 * Description : Send a command and argument to the SD Card.
 * 
 * Arguments   : cmd     SD Card command. See sd_spi_cmds.h.
 * 
 *               arg     32-bit argument to be sent with the SD command.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */
void sd_sendCommand(uint8_t cmd, uint32_t arg);

/*
 * ----------------------------------------------------------------------------
 *                                                              GET R1 RESPONSE
 * 
 * Description : Gets the R1 response from the SD card after it has been sent 
 *               an SD command.
 * 
 * Arguments   : void
 * 
 * Returns     : R1 response flag(s). See SD_SPI_BASE.H.
 * 
 * Notes       : 1) Always call immediately after sd_sendCommand().
 *               
 *               2) Pass the return value to sd_printR1() to print the value.
 * 
 *               3) R1_TIMEOUT is returned if the SD Card did not return an R1
 *                  response. 
 * ----------------------------------------------------------------------------
 */
uint8_t sd_getR1(void);

/*
 * ----------------------------------------------------------------------------
 *                                                      PRINT R1 RESPONSE FLAGS
 * 
 * Description : Prints the R1 response flag(s) returned by sd_getR1().
 * 
 * Arguments   : r1     The R1 response flag(s) byte returned by sd_getR1().
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */
void sd_printR1(uint8_t r1);

/*
 * ----------------------------------------------------------------------------
 *                                          PRINT INITIALIZATION RESPONSE FLAGS
 * 
 * Description : Prints Initialization Error Flag portion of the 
 *               sd_spiModeInit() response.
 * 
 * Arguments   : initResp     The Initialization Error Response returned by the
 *                            initialization routine, sd_spiModeInit().
 * 
 * Returns     : void
 * 
 * Notes       : This will only interpret bits 8 to 19 of the sd_spiModeInit() 
 *               function's returned value. The entire returned value can be 
 *               passed to this function without issue. Bits 0 to 7 correspond
 *               to the R1 Response portion of the Initialization Response. To 
 *               read the R1 portion pass initResp to sd_printR1().
 * ----------------------------------------------------------------------------
 */
void sd_printInitError(uint32_t initErr);

#endif //SD_SPI_H
