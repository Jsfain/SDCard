/*
Provides several functions and macros to be used for base-level interaction
between an SD Card and an AVR microcontroller when operating in SPI mode.

File    : SD_SPI_BASE.H
Version : 0.0.0.1 
Author  : Joshua Fain
Target  : ATMega1280
License : MIT
Copyright (c) 2020-2021

*/

#ifndef SD_SPI_H
#define SD_SPI_H

/*! 
 \file sd_spi_base.h
 \brief Interace for base-level interaction between an SD Card and AVR
 microcontroller operating in SPI Mode.
 
 This interface file provides several functions and macros that can be used
 for base-level interaction between an SD Card and an AVR microcontroller 
 when operating in SPI mode.

 To use:

 1) Create an instance of \c CardTypeVersion , *typedef* \c CTV.

 2) Pass this \c CTV instance to the intialization routine \c sd_spiModeInit() 
 which will initialize the card into SPI mode and set the member so the \c CTV
 instance to the correct values according to the card. See code sample below.

 \warning This has only been tested against 2GB SDSC and 4GB SDHC micro-SD
 cards.

 \code
 CTV ctv;
 initResp = sd_spiModeInit(&ctv);
 \endcode
*/


/** 
 \defgroup sd_base <sd_spi_base.h>
 For base-level interaction between an SD Card and AVR microcontroller 
 operating in SPI Mode.
*/

#include "sd_spi_cmds.h"

/*
*******************************************************************************
*******************************************************************************
 *                     
 *                                   MACROS
 *  
*******************************************************************************
*******************************************************************************
*/

// ----------------------------------------------------------------------------
//                                                         Host Capcity Support
/*!
 \ingroup sd_base
 \brief **Host Capacity Support**
 
 0 - host only supports SDSC.
 
 1 - host also supports SDHC/SDXC.

 \note Recommend leaving this set to 1 to support both types. In the current
 implementation nothing is lost by having the host support both card types, 
 but if the card is type SDHC/SDXC and this value is set to 0 (SDSC only), the
 host will fail to initialize the card.
*/
#define HCS 1



// ----------------------------------------------------------------------------
//                                                                 Block Length
/*!
 \ingroup sd_base
 \details **Block Length**
 
 Defines the SD card block length (in bytes) to be assumed by the host.
 \warning Always set to 512. For SDHC/SDXC cards, block length is fixed at 512. 
 For SDSC cards, it is possible to change block length on the SD card with the 
 SET_BLOCKLEN (CMD16) command, but setting this value to anything other than 
 512 using this implementation should not be done; It will fail and/or produce
 unexpected results.
*/
#define BLOCK_LEN 512 //bytes



// ----------------------------------------------------------------------------
//                                                      Chip Select (De-)Assert
/*!
 \ingroup sd_base
 \brief **Chip Select Assert**
 
 Asserting the chip select pin (i.e. bringing it low) signals to the SD card
 operating in SPI mode to prepare for communincation. This pin must remain low 
 until all responses, data, tokens, etc... have been sent/received for a given
 command. 
 \attention This implementation uses pin SS0 (defined in SPI.H) for the chip
 select pin, however, the SS1 pin is also defined in the SPI interface file for
 attaching a second device to the SPI port. Therefore, when asserting the SD 
 card's chip select pin, the MACRO also ensures any device attached to SS1 will
 be inactive by de-asserting the SS1 pin.
*/
#define CS_SD_LOW    SPI_PORT = ((SPI_PORT & ~(1 << SS0)) | (1 << SS1));
/*!
 \ingroup sd_base
 \brief **Chip Select De-Assert**

 De-Assert the chip select pin (i.e. bring it high). This signals to the SD 
 card operating in SPI mode to stop communincation. This pin should only be set
 high after the final response, data, token, etc... has been sent/received for
 a given SD card command. 
*/
#define CS_SD_HIGH   SPI_PORT |=  (1 << SS0);



// ----------------------------------------------------------------------------
//                                                            R1 Response Flags
/*! \ingroup sd_base
 *  R1 Response Flag */
#define IN_IDLE_STATE           0x01
/*! \ingroup sd_base
 R1 Response Flag */
#define ERASE_RESET             0x02
/*! \ingroup sd_base
 R1 Response Flag */
#define ILLEGAL_COMMAND         0x04
/*! \ingroup sd_base
 R1 Response Flag */
#define COM_CRC_ERROR           0x08
/*! \ingroup sd_base
 R1 Response Flag */
#define ERASE_SEQUENCE_ERROR    0x10
/*! \ingroup sd_base
 R1 Response Flag */
#define ADDRESS_ERROR           0x20
/*! \ingroup sd_base
 R1 Response Flag */
#define PARAMETER_ERROR         0x40
/*! \ingroup sd_base
 R1 Response Flag 
 \note This flag is not a true R1 response from the SD card but is a flag set
 by the sd_getR1() function if the SD card does not return an R1 response in
 the allotted amount of time. */
#define R1_TIMEOUT              0x80



// ----------------------------------------------------------------------------
//                                                   Initialization Error Flags
//
// Flags returned in bits 8 to 19 of the SD card initialization
// routine, sd_spiModeInit(). Note the lowest byte in this function's
// returned response will be the most recent R1 response, therefore 
// these flags begin at byte 2.
/*! \ingroup sd_base
 Initialization Error Flag */
#define FAILED_GO_IDLE_STATE    0x00100   //CMD0
/*! \ingroup sd_base
 Initialization Error Flag */
#define FAILED_SEND_IF_COND     0x00200   //CMD8
/*! \ingroup sd_base
 Initialization Error Flag */
#define UNSUPPORTED_CARD_TYPE   0x00400
/*! \ingroup sd_base
 Initialization Error Flag */
#define FAILED_CRC_ON_OFF       0x00800   //CMD59
/*! \ingroup sd_base
 Initialization Error Flag */
#define FAILED_APP_CMD          0x01000   //CMD55
/*! \ingroup sd_base
 Initialization Error Flag */
#define FAILED_SD_SEND_OP_COND  0x02000   //ACMD41
/*! \ingroup sd_base
 Initialization Error Flag */
#define OUT_OF_IDLE_TIMEOUT     0x04000
/*! \ingroup sd_base
 Initialization Error Flag */
#define FAILED_READ_OCR         0x08000   //CMD58
/*! \ingroup sd_base
 Initialization Error Flag */
#define POWER_UP_NOT_COMPLETE   0x10000



// ----------------------------------------------------------------------------
//                                                                   Card Types
/*! 
 \ingroup sd_base
 \details **Card Type** 
 If \c type member of CardTypeVersion is set to SDHC, then the initialization 
 routine, sd_spiModeInit(), determined that the card type is a high\extended
 capacity card, and should be block addressed */
#define SDHC 1 // high (or extended SDXC) capacity
/*! 
 \ingroup sd_base
 \details **Card Type** 
 If \c type member of CardTypeVersion is set to SDSC, then the initialization 
 routine, sd_spiModeInit(), determined that the card type is a standard
 capacity card, and should be byte addressed */
#define SDSC 0 // standard capacity





/*
*******************************************************************************
*******************************************************************************
 *                     
 *                                   STRUCTS
 *  
*******************************************************************************
*******************************************************************************
*/



// ----------------------------------------------------------------------------
//                                                         Card Types / Version
/*! 
 \ingroup sd_base
 \details Struct used to hold the card's type and version.
 \param uint8_t version : Either 1 or 2.
 \param uint8_t type : Either be SDSC or SDHC.
 \attention 1) An instance of this struct should only be set by the
 initialization routine, sd_spiModeInit(CTV *ctv). 
 \attention 2) Although it is expected that this implementation will support 
 version 1 cards, only version 2 cards have been tested. 
 \note The value of type will determine how the card is addressed. Is it 
 * **Byte** or **Block** addressable?
*/
typedef struct CardTypeVersion {
    uint8_t version;
    uint8_t type;
} CTV;





/*
*******************************************************************************
*******************************************************************************
 *                     
 *                           FUNCTION DECLARATIONS
 *  
*******************************************************************************
*******************************************************************************
*/

/*!
 * \ingroup sd_base
 * \details Initializes the SD card to operate in spi mode and sets members
 * of the CTV struct. This function must be called first before implementing
 * any other part of the AVR-SD Card module.
 * \param CTV *ctv   :pointer to a CTV instance. The members of this instance
 * will be set by this function.
 * \returns uint32_t Initialization Error Response.
 * \note The returned *Initialization Error Response* will include an 
 * *Initialization Error Flag* and the most recent *R1 Response Flag*. To read
 * the R1 response, pass this return value to sd_printR1(err). To read the 
 * Initialization Error Flag, pass it to sd_printInitError(err).
 */
uint32_t 
sd_spiModeInit (CTV* ctv);



/*!
 * \ingroup sd_base
 * \details Sends a single byte to the SD Card via the SPI port. This function
 * along with sd_receiveByteSPI(), are the SPI port interfacing functions.
 * \param uint8_t byte - byte packed that will be sent to the SD card.
 * \note Call as many times as required to send the complete data packet, token
 * command, etc...
 */
void 
sd_sendByteSPI (uint8_t byte);



/*!
 * \ingroup sd_base
 * \details Gets a single byte received from the SD card via the SPI port.
 * This function, along with sd_sendByteSPI() are the SPI port interfacing 
 * functions.
 * \returns uint8_t byte received by the microcontroller from the SD card via
 * the SPI port.
 * \note Call as many times as necessary to receive the complete data packet, 
 * token, response, etc... sent by the SD card.
 */
uint8_t 
sd_receiveByteSPI (void);



/*!
 * \ingroup sd_base
 * \details Send a command and argument to the SD Card.
 * \param uint8_t cmd - one of the available SD Card SPI mode commands.
 * \param uint32_t arg - the argument to send with the command.
 * \note This function will call a "private" function that calculates the CRC7
 * to be sent with the SD Card. By default the CRC7 is only needed for the 
 * second command sent in the intialization routine (CMD8), but CRC can be
 * switched on by calling the CRC_ON_OFF command.
 */
void 
sd_sendCommand (uint8_t cmd, uint32_t arg);



/*!
 * \ingroup sd_base
 * \details Gets the R1 response after sending a cmd/arg to the SD card.
 * \returns uint8_t R1 response flag.
 * \note 1) An R1 response is the first byte returned by the SD card after
 * sending any command, therefore this function should be called immediately 
 * after sending any command.
 * \note 2) A true R1 response from an SD card will only set bits 0 to 6 in 
 * the response byte, bit 7 is reserved and always set to zero. As such, bit 8
 * may be set by this function as R1_TIMEOUT to indicate that the R1 response 
 * was not received in the expected amount of time/clock cycles.
 * \note 3) Read the R1 Response by passing the returned value to 
 * sd_printR1(r1).
*/
uint8_t 
sd_getR1 (void);



/*!
 * \ingroup sd_base
 * \details Prints the R1 response flag(s) returned by sd_getR1(r1).
 * \param uint8_t r1 - R1 Response
 * \note prints the R1 response returned by sd_getR1(r1).
 */
void 
sd_printR1 (uint8_t r1);



/*!
 * \ingroup sd_base
 * \details Prints the *Initialization Error Flag* portion of the response
 * returned by sd_spiModeInit().
 * \param uint8_t initErr - Initialization Response
 * \note 1) Prints the *Initialization Error Flag* returned by the
 * initialization routine, sd_spiModeInit().
 * \note 2) Only bits 8-19 of the value returned by sd_spiModeInit() are
 * the *Initialization Error Flags*, bits 0 to 7 correspond to the R1 Response
 * portion of the *Initialization Response*. Passing the entire *Initialization
 * Respose* is valid, but only *Initialization Error Flags* will be printed. To
 * print the R1 Response portion, pass the *Initialization Response* to 
 * sd_printR1(r1).
 */
void 
sd_printInitError (uint32_t initErr);


#endif //SD_SPI_H
