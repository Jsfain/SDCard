/******************************************************************************
 * File    : SD_SPI_BASE.H
 * Version : 0.0.0.1 
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : MIT
 * Copyright (c) 2020-2021
 * 
 * Description:
 * Provides several functions and macros for basic interaction between an SD
 * SD Card and an AVR microcontroller when operating in SPI mode.
 *****************************************************************************/


#ifndef SD_SPI_H
#define SD_SPI_H



#include "sd_spi_cmds.h"




/******************************************************************************
 ******************************************************************************
 *                     
 *                                   MACROS
 *  
 ******************************************************************************
 ******************************************************************************/


/* ----------------------------------------------------------------------------
 *                                                                 BLOCK LENGTH
 *
 * DESCRIPTION: 
 * The SD card block length (in bytes) assumed by the host. 
 *       
 * WARNING: 
 * This should always be set to 512. If not the module will produce unexpected
 * results and/or fail.
 --------------------------------------------------------------------------- */
#define BLOCK_LEN 512



/* ----------------------------------------------------------------------------
 *                                                                  CHIP SELECT
 * 
 * NOTES: 
 * 1) Assert by setting CS low. De-Assert by setting CS high.
 * 1) SSO, defined in SPI.H, is used here as the CS pin. 
 * 2) SPI.H also defines a second SS pin (SS1) for connecting multiple devices
 *    to the SPI port of the AVR. Therefore, this MACRO must also ensure the
 *    SS1 pin is de-asserted (set high) when SS0 is asserted (set low).
 * ------------------------------------------------------------------------- */
#define CS_SD_LOW    SPI_PORT = ((SPI_PORT & ~(1 << SS0)) | (1 << SS1));
#define CS_SD_HIGH   SPI_PORT |=  (1 << SS0);



/* ----------------------------------------------------------------------------
 *                                                            R1 RESPONSE FLAGS
 * 
 * DESCRIPTION: 
 * Flags returned by sd_getR1().
 * 
 * NOTES: 
 * 1) With the exception of R1_TIMEOUT, these flags correspond to the first 
 *    byte returned by the SD card in response to any command.
 * 2) R1_TIMEOUT will be set in the sd_getR1() return value if the SD card does
 *    not send an R1 response after set amount of time.
 * ------------------------------------------------------------------------- */
#define IN_IDLE_STATE           0x01
#define ERASE_RESET             0x02
#define ILLEGAL_COMMAND         0x04
#define COM_CRC_ERROR           0x08
#define ERASE_SEQUENCE_ERROR    0x10
#define ADDRESS_ERROR           0x20
#define PARAMETER_ERROR         0x40
#define R1_TIMEOUT              0x80



/* ----------------------------------------------------------------------------
 *                                                   INITIALIZATION ERROR FLAGS
 * 
 * DESCRIPTION: 
 * Flags returned in bits 8 to 19 by sd_spiModeInit().
 *        
 * NOTES: 
 * The lowest byte returned by sd_spiModeInit() is the most recent R1 Response.
 * ------------------------------------------------------------------------- */
#define FAILED_GO_IDLE_STATE    0x00100   // CMD0
#define FAILED_SEND_IF_COND     0x00200   // CMD8
#define UNSUPPORTED_CARD_TYPE   0x00400
#define FAILED_CRC_ON_OFF       0x00800   // CMD59
#define FAILED_APP_CMD          0x01000   // CMD55
#define FAILED_SD_SEND_OP_COND  0x02000   // ACMD41
#define OUT_OF_IDLE_TIMEOUT     0x04000
#define FAILED_READ_OCR         0x08000   // CMD58
#define POWER_UP_NOT_COMPLETE   0x10000



/* ----------------------------------------------------------------------------
 *                                                                   CARD TYPES
 * 
 * NOTES:
 * 1) High / extended capacity (SDHC) cards are block addressable.
 * 2) Standard capacity (SDSC) cards are byte addressable. 
 * ------------------------------------------------------------------------- */
#define SDHC   1 
#define SDSC   0



/* ----------------------------------------------------------------------------
 *                                                        HOST CAPACITY SUPPORT
 * 
 * DESCRIPTION: 
 * Specifies the card type(s) that the host will support.
 *        
 * NOTE: 
 * 1) Set to either SDHC or SDSC.
 * 2) Advise setting to SDHC, which will also support SDSC.
 * 3) Setting to SDSC will cause the host to only support SDSC cards. 
 * ------------------------------------------------------------------------- */
#define HOST_CAPACITY_SUPPORT  SDHC





/******************************************************************************
 ******************************************************************************
 *                     
 *                                   STRUCTS
 *  
 ******************************************************************************
 ******************************************************************************/


/* ----------------------------------------------------------------------------
 *                                                        CARD TYPE and VERSION
 * 
 * MEMBERS:
 * 1) uint8_t version - SD Card's version. Either 1 or 2.
 * 2) uint8_t type    - SD Card's type. Either SDSC or SDHC.
 * 
 * NOTES:
 *  - The members of this struct should only be set by the initialization 
 *    routine sd_spiModeInit(*ctv).
 *  - The value of 'type' can be used to determine how the card is addressed by
 *    any command accessing data blocks.
 * 
 * WARNINGS:
 * Only version 2 cards have been tested.
 * ------------------------------------------------------------------------- */
typedef struct CardTypeVersion {
    uint8_t version;
    uint8_t type;
} CTV;






/******************************************************************************
 ******************************************************************************
 *                     
 *                           FUNCTION DECLARATIONS
 *  
 ******************************************************************************
 *****************************************************************************/


/*-----------------------------------------------------------------------------
 *                                                           INITIALIZE SD CARD
 * 
 * DESCRIPTION: 
 * Initializes an SD card into SPI mode and sets members of the CTV instance. 
 * 
 * ARGUMENTS: 
 * CTV *ctv  - ptr to a CTV instance whose members will be set here.
 * 
|* RETURN: 
 * uint32_t - Initialization Error Response.
 * 
 * NOTES: 
 * The returned value contains the Initialization Error Flag(s) in bits 8 to 19
 * and the most recent R1 Response in the lowest byte.
 * ------------------------------------------------------------------------- */
uint32_t 
sd_spiModeInit (CTV* ctv);



/*-----------------------------------------------------------------------------
 *                                                                    SEND BYTE
 * 
 * DESCRIPTION: 
 * Sends a single byte to the SD card via the SPI port.
 * 
 * ARGUMENT:
 * uint8_t byte  - 8-bit packet that will be sent to the SD Card via SPI.
 * 
 * RETURN:
 * void
 * 
 * NOTES: 
 * 1) Call this function as many times as required to send the complete data 
 *    packet, token, command, etc...
 * 2) This, along with sd_receiveByteSPI(), are the SPI interfacing functions.
 * ------------------------------------------------------------------------- */
void 
sd_sendByteSPI (uint8_t byte);



/*-----------------------------------------------------------------------------
 *                                                                 RECEIVE BYTE
 * 
 * DESCRIPTION: 
 * Receives and returns a single byte from the SD card via the SPI port.
 * 
 * ARGUMENT:
 * void
 * 
 * RETURN:
 * uint8_t - Single byte received from the SD card.
 * 
 * NOTE:
 * 1) Call this function as many times as necessary to get the complete data
 *    packet, token, error response, etc... received by the SD card.
 * 2) This, along with sd_sendByteSPI(), are the SPI interfacing functions.
 * ------------------------------------------------------------------------- */
uint8_t 
sd_receiveByteSPI (void);



/*-----------------------------------------------------------------------------
 *                                                                 SEND COMMAND
 * 
 * DESCRIPTION: 
 * Send a command and argument to the SD Card.
 * 
 * ARGUMENT:
 * 1) uint8_t  cmd - SD Card command. See sd_spi_cmds.h
 * 2) uint32_t arg - 32-bit argument to be sent with the command. 
 * 
 * RETURN:
 * void
 * ------------------------------------------------------------------------- */
void 
sd_sendCommand (uint8_t cmd, uint32_t arg);



/*-----------------------------------------------------------------------------
 *                                                        GET R1 RESPONSE FLAGS
 * 
 * DESCRIPTION: 
 * Gets the R1 response from the SD card after sending it a SD command.
 * 
 * ARGUMENT:
 * void 
 * 
 * RETURN:
 * uint8_t - R1 response flags.
 * 
 * NOTES:
 * 1) Always call this function immediately after sd_sendCommand().
 * 1) Pass the return value to sd_printR1(err) to print the R1 response.
 * 2) If R1_TIMEOUT is returned, this indicates that the SD Card did not
 *    return an R1 response within a specified amount of time.
 * ------------------------------------------------------------------------- */
uint8_t 
sd_getR1 (void);



/*-----------------------------------------------------------------------------
 *                                                      PRINT R1 RESPONSE FLAGS
 * 
 * DESCRIPTION: 
 * Prints the R1 response flag(s) returned by sd_getR1().
 * 
 * ARGUMENT:
 * uint8_t r1 - R1 response flags byte returned by sd_getR1(). 
 * 
 * RETURN:
 * void
 * ------------------------------------------------------------------------- */
void 
sd_printR1 (uint8_t r1);



/*-----------------------------------------------------------------------------
 *                                          PRINT INITIALIZATION RESPONSE FLAGS
 * 
 * DESCRIPTION: 
 * Prints the Initialization Error Flag portion of the response returned by 
 * sd_spiModeInit().
 *  
 * ARGUMENT:
 * uint32_t initErr - The Initialization Error Response returned by the 
 *                    initialization routine, sd_spiModeInit().
 * 
 * RETURN:
 * void
 * 
 * NOTES:
 * Though this will only print bits 8 to 19 of the sd_spiModeInit() function's
 * returned value (the Initialization Error Response), the entire returned
 * value can be passed to this function without issue. Bits 0 to 7 correspond
 * to the R1 Response portion of the Initialization Response. To read this
 * portion pass the Initialization Error Response to sd_printR1(initErr).
 * ------------------------------------------------------------------------- */
void 
sd_printInitError (uint32_t initErr);



#endif //SD_SPI_H
