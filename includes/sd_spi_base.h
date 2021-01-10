/*
File    : SD_SPI_BASE.H
Version : 0.0.0.1 
Author  : Joshua Fain
Target  : ATMega1280
License : MIT
Copyright (c) 2020-2021
 
DESCRIPTION:
Provides several functions and macros to be used for base-level interaction
between an SD Card and an AVR microcontroller when operating in SPI mode.
*/

#ifndef SD_SPI_H
#define SD_SPI_H

/*! 
 @file sd_spi_base.h
 @brief Interace for base-level interaction between an SD Card and AVR
 operating in SPI Mode.
 
 This interface file provides several functions and macros that can be used
 for base-level interaction between an SD Card and an AVR microcontroller 
 when operating in SPI mode.

 To use:

 1) Create an instance of \c CardTypeVersion , *typedef* \c CTV.

 2) Pass this \c CTV instance to the intialization routine \c sd_spiModeInit() 
 which will initialize the card into SPI mode and set the member so the \c CTV
 instance to the correct values according to the card. See code sample below.

 @code
 CTV ctv;
 initResp = sd_spiModeInit(&ctv);
 @endcode
*/


/** 
 @defgroup sd_base <sd_spi_base.h>
 For base-level interaction between an SD Card and AVR microcontroller 
 operating in SPI Mode.
*/

#include "../includes/sd_spi_cmds.h"

/*
*******************************************************************************
*******************************************************************************
 *                     
 *                                   MACROS
 *  
*******************************************************************************
*******************************************************************************
*/

// *********** Host Capcity Support

// 0 = host only supports SDSC. 
// 1 = host also supports SDHC or SDXC.
/*!
* \ingroup sd_base
* \details **Host Capacity Support**
* 
* 0 : host only supports SDSC.
* 
* 1 : host also supports SDHC or SDXC.
*/
#define HCS 1



// *********** Block Length. 

// Currently only a value of 512 is valid.
/*!
* \ingroup sd_base
* \details **Block Length**
* 
* Defines the SD card block length that will be assumed by this implmentation.
* \warning Always set to 512. Any other value will fail and/or produce
unexpected results.
*/
#define BLOCK_LEN 512



// *********** Define CS for SPI and settings

// SPI.C/H currently allows for two SS pins (multiple SPI devices).
// This SD Card Module is setup to use SS0 as the chip-select pin.
/*!
* \ingroup sd_base
* \details **Chip Select Assert**
* 
* Asserting the chip select pin (i.e. bringing it low) signals to the
* SD Card operating in SPI mode to prepare for communincation. This pin must 
* remain low until all responses, data, tokens, etc... have been sent/received
* for a given command. 
* \attention This implementation uses pin SS0 (defined in SPI.H) for the chip
* select pin.  The SS1 pin is also defined in the SPI interface file for
* attaching a second device to the SPI port. This MACRO therefore sets the SS1
* pin high to de-assert it when asserting SS0.
*/
#define CS_SD_LOW    SPI_PORT = ((SPI_PORT & ~(1 << SS0)) | (1 << SS1));
/*!
* \ingroup sd_base
* \details **Chip Select De-Assert**

* De-Assert the chip select pin (i.e. bring it high). This signals to
* the SD Card operating in SPI mode to stop communincation with the SD Card. 
* This pin should only be set high after the final responses, data, token,
* etc... has been sent/received for a given SD Card command. 
*/
#define CS_SD_HIGH   SPI_PORT |=  (1 << SS0);


// ********** R1 Response Flags

// SD Card R1 response flags, except R1_TIMEOUT. This flag
// is used here to indicate when the SD Card does not return
// a valid R1 response in a specified amount of time.
/*! \ingroup sd_base
 *  R1 Response Flag */
#define IN_IDLE_STATE           0x01
/*! \ingroup sd_base
 *  R1 Response Flag */
#define ERASE_RESET             0x02
/*! \ingroup sd_base
 *  R1 Response Flag */
#define ILLEGAL_COMMAND         0x04
/*! \ingroup sd_base
 *  R1 Response Flag */
#define COM_CRC_ERROR           0x08
/*! \ingroup sd_base
 *  R1 Response Flag */
#define ERASE_SEQUENCE_ERROR    0x10
/*! \ingroup sd_base
 *  R1 Response Flag */
#define ADDRESS_ERROR           0x20
/*! \ingroup sd_base
 *  R1 Response Flag */
#define PARAMETER_ERROR         0x40
/*! \ingroup sd_base
 *  R1 Response Flag */
#define R1_TIMEOUT              0x80



// *********** Initialization Error Flags

// Flags returned in bits 8 to 19 of the SD card initialization
// routine, sd_spiModeInit(). Note the lowest byte in this function's
// returned response will be the most recent R1 response, therefore 
// these flags begin at byte 2.
/*! \ingroup sd_base
 *  Initialization Error Flag */
#define FAILED_GO_IDLE_STATE    0x00100   //CMD0
/*! \ingroup sd_base
 *  Initialization Error Flag */
#define FAILED_SEND_IF_COND     0x00200   //CMD8
/*! \ingroup sd_base
 *  Initialization Error Flag */
#define UNSUPPORTED_CARD_TYPE   0x00400
/*! \ingroup sd_base
 *  Initialization Error Flag */
#define FAILED_CRC_ON_OFF       0x00800   //CMD59
/*! \ingroup sd_base
 *  Initialization Error Flag */
#define FAILED_APP_CMD          0x01000   //CMD55
/*! \ingroup sd_base
 *  Initialization Error Flag */
#define FAILED_SD_SEND_OP_COND  0x02000   //ACMD41
/*! \ingroup sd_base
 *  Initialization Error Flag */
#define OUT_OF_IDLE_TIMEOUT     0x04000
/*! \ingroup sd_base
 *  Initialization Error Flag */
#define FAILED_READ_OCR         0x08000   //CMD58
/*! \ingroup sd_base
 *  Initialization Error Flag */
#define POWER_UP_NOT_COMPLETE   0x10000



// ********** Card Type
/*! \ingroup sd_base
 *  \details **Card Type** 
 *  If \c type member of CardTypeVersion is set to SDHC then the card is a high
 *  capacity\exteneded capacity card, and should be block addressed */
#define SDHC 1 // high (or extended SDXC) capacity
/*! \ingroup sd_base
 *  \details **Card Type** 
 *  If \c type member of CardTypeVersion is set to SDSC then the card is a
 *  standard capacity\exteneded capacity card, and should be block addressed */
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


// ********** Card Type / Version 
/*! \ingroup sd_base
 *  \details Holds the card's type and version.
 *  \param uint8_t version - set to SD card's version.
 *  \param uint8_t type - set to SD card's type.
 *  \attention An instance of this struct should only be set by the
 *  initialization routine, sd_spiModeInit(CTV *ctv). 
 *  \note The value of type will determine how the card is addressed - Byte or
 *  Block addressable?
 *  \note The instance should only be set by the initialization routine.
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
 * @ingroup sd_base
 * @details Initializes the SD card to operate in spi mode and sets members
 * of the CTV struct. This function must be called first before implementing
 * any other part of the AVR-SD Card module.
 * @param CTV *ctv   :pointer to a CTV instance. The members of this instance
 * will be set by this function.
 * @returns uint32_t Initialization Error Response.
 * @note The returned *Initialization Error Response* will include an 
 * *Initialization Error Flag* and the most recent *R1 Response Flag*. To read
 * the R1 response, pass this return value to sd_printR1(err). To read the 
 * Initialization Error Flag, pass it to sd_printInitError(err).
 */
uint32_t 
sd_spiModeInit (CTV* ctv);



/*!
 * @ingroup sd_base
 * @details Sends a single byte to the SD Card via the SPI port. This function
 * along with sd_receiveByteSPI(), are the SPI port interfacing functions.
 * @param uint8_t byte - byte packed that will be sent to the SD card.
 * @note Call as many times as required to send the complete data packet, token
 * command, etc...
 */
void 
sd_sendByteSPI (uint8_t byte);



/*!
 * @ingroup sd_base
 * @details Gets a single byte received from the SD card via the SPI port.
 * This function, along with sd_sendByteSPI() are the SPI port interfacing 
 * functions.
 * @returns uint8_t byte received by the microcontroller from the SD card via
 * the SPI port.
 * @note Call as many times as necessary to receive the complete data packet, 
 * token, response, etc... sent by the SD card.
 */
uint8_t 
sd_receiveByteSPI (void);



/*!
 * @ingroup sd_base
 * @details Send a command and argument to the SD Card.
 * @param uint8_t cmd - one of the available SD Card SPI mode commands.
 * @param uint32_t arg - the argument to send with the command.
 * @note This function will call a "private" function that calculates the CRC7
 * to be sent with the SD Card. By default the CRC7 is only needed for the 
 * second command sent in the intialization routine (CMD8), but CRC can be
 * switched on by calling the CRC_ON_OFF command.
 */
void 
sd_sendCommand (uint8_t cmd, uint32_t arg);



/*!
 * @ingroup sd_base
 * @details Gets the R1 response after sending a cmd/arg to the SD card.
 * @returns uint8_t R1 response flag.
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
 * @ingroup sd_base
 * @details Prints the R1 response flag(s) returned by sd_getR1(r1).
 * @param uint8_t r1 - R1 Response
 * @note prints the R1 response returned by sd_getR1(r1).
 */
void 
sd_printR1 (uint8_t r1);



/*!
 * @ingroup sd_base
 * @details Prints the *Initialization Error Flag* portion of the response
 * returned by sd_spiModeInit().
 * @param uint8_t initErr - Initialization Response
 * @note 1) Prints the *Initialization Error Flag* returned by the
 * initialization routine, sd_spiModeInit().
 * @note 2) Only bits 8-19 of the value returned by sd_spiModeInit() are
 * the *Initialization Error Flags*, bits 0 to 7 correspond to the R1 Response
 * portion of the *Initialization Response*. Passing the entire *Initialization
 * Respose* is valid, but only *Initialization Error Flags* will be printed. To
 * print the R1 Response portion, pass the *Initialization Response* to 
 * sd_printR1(r1).
 */
void 
sd_printInitError (uint32_t initErr);


#endif //SD_SPI_H

