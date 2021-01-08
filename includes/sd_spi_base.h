/*
*******************************************************************************
*                              AVR-SD CARD MODULE
*
* File    : SD_SPI_BASE.H
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
* License : MIT
* Copyright (c) 2020
* 
*
* DESCRIPTION:
* Interface for the basic SPI mode SD card functions for physical interaction 
* of an AVR microcontroller with an SD card operating in SPI Mode.
*******************************************************************************
*/


#ifndef SD_SPI_H
#define SD_SPI_H





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
#define HCS 1



// *********** Block Length. 

// Currently only a value of 512 is valid.
#define BLOCK_LEN 512



// *********** Define CS for SPI and settings

// SPI.C/H currently allows for two SS pins (multiple SPI devices).
// This SD Card Module is setup to use SS0 as the chip-select pin.
#define CS_SD_LOW    SPI_PORT = ((SPI_PORT & ~(1 << SS0)) | (1 << SS1));
#define CS_SD_HIGH   SPI_PORT |=  (1 << SS0);



// ********** SD card commands

// Commands available to an SD Card operating in SPI mode
#define GO_IDLE_STATE               0       //CMD0
#define SEND_OP_COND                1       //CMD1
#define SWITCH_FUNC                 6       //CMD6
#define SEND_IF_COND                8       //CMD8
#define SEND_CSD                    9       //CMD9
#define SEND_CID                    10      //CMD10
#define STOP_TRANSMISSION           12      //CMD12
#define SEND_STATUS                 13      //CMD13
#define SET_BLOCKLEN                16      //CMD16
#define READ_SINGLE_BLOCK           17      //CMD17
#define READ_MULTIPLE_BLOCK         18      //CMD18
#define WRITE_BLOCK                 24      //CMD24
#define WRITE_MULTIPLE_BLOCK        25      //CMD25
#define PROGRAM_CSD                 27      //CMD27
#define SET_WRITE_PROT              28      //CMD28
#define CLR_WRITE_PROT              29      //CMD29
#define SEND_WRITE_PROT             30      //CMD30
#define ERASE_WR_BLK_START_ADDR     32      //CMD32
#define ERASE_WR_BLK_END_ADDR       33      //CMD33
#define ERASE                       38      //CMD38
#define LOCK_UNLOCK                 42      //CMD42
#define APP_CMD                     55      //CMD55
#define GEN_CMD                     56      //CMD56
#define READ_OCR                    58      //CMD58
#define CRC_ON_OFF                  59      //CMD59
// Application Specific Commands. To activate, first call APP_CMD.
#define SD_STATUS                   13      //ACMD13
#define SEND_NUM_WR_BLOCKS          22      //ACMD22
#define SET_WR_BLK_ERASE_COUNT      23      //ACMD23
#define SD_SEND_OP_COND             41      //ACMD41
#define SET_CLR_CARD_DETECT         42      //ACMD42
#define SEND_SCR                    51      //ACMD51



// ********** R1 Response Flags

// SD Card R1 response flags, except R1_TIMEOUT. This flag
// is used here to indicate when the SD Card does not return
// a valid R1 response in a specified amount of time.
#define IN_IDLE_STATE           0x01
#define ERASE_RESET             0x02
#define ILLEGAL_COMMAND         0x04
#define COM_CRC_ERROR           0x08
#define ERASE_SEQUENCE_ERROR    0x10
#define ADDRESS_ERROR           0x20
#define PARAMETER_ERROR         0x40
#define R1_TIMEOUT              0x80



// *********** Initialization Error Flags

// Flags returned in bits 8 to 19 of the SD card initialization
// routine, sd_spiModeInit(). Note the lowest byte in this function's
// returned response will be the most recent R1 response, therefore 
// these flags begin at byte 2.
#define FAILED_GO_IDLE_STATE    0x00100   //CMD0
#define FAILED_SEND_IF_COND     0x00200   //CMD8
#define UNSUPPORTED_CARD_TYPE   0x00400
#define FAILED_CRC_ON_OFF       0x00800   //CMD59
#define FAILED_APP_CMD          0x01000   //CMD55
#define FAILED_SD_SEND_OP_COND  0x02000   //ACMD41
#define OUT_OF_IDLE_TIMEOUT     0x04000
#define FAILED_READ_OCR         0x08000   //CMD58
#define POWER_UP_NOT_COMPLETE   0x10000



// ********** Card Type
#define SDHC 1 // high (or extended SDXC) capacity
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

// Holds the card's type and version. This is most important for 
// determining how to address the SD Card. Block or Byte addressable.
// The instance should only be set by the initialization routine.
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

/*
-------------------------------------------------------------------------------
|                      INITIALIZE AN SD CARD INTO SPI MODE 
|                                        
| Description : Initializes the SD card to operate in spi mode and sets members
|               of the CTV struct. This function must be called first before 
|               implementing any other part of the AVR-SD Card module.
|
| Argument    : *ctv    - ptr to a CTV instance. The members of this instance
|                         will be set by this function. The 'type' setting is
|                         critical for block address handling.
|
| Return      : Initialization Error Response. This will include an 
|               Initialization Error Flag and the most recent R1 Response Flag.
|               To read the R1 response, pass it to sd_printR1(err). To read
|               the Initialization Error Flag, call sd_printError(err).
-------------------------------------------------------------------------------
*/

uint32_t 
sd_spiModeInit (CTV * ctv);



/*
-------------------------------------------------------------------------------
|                             SEND BYTE TO THE SD CARD
|                                        
| Description : Sends a single byte to the SD Card via the SPI port. This
|               function, along with sd_receiveByteSPI(), are the SPI port 
|               interfacing functions.
|
| Argument    : byte   - byte packet that will be sent to the SD Card via SPI.
|
| Note       : Call as many times as required to send the complete data packet,
|              token, command, etc...
-------------------------------------------------------------------------------
*/

void 
sd_sendByteSPI (uint8_t byte);



/*
-------------------------------------------------------------------------------
|                             RECEIVE BYTE FROM THE SD CARD
|                                        
| Description : Gets a single byte sent to the SD Card sent by a device via SPI.
|               This function, along with sd_sendByteSPI() are the SPI port 
|               interfacing functions. 
|
| Return      : byte that will be received by the SD card via the SPI port.
|
| Note        : Call as many times as necessary to get the complete data
|               packet, token, error response, etc... received by the SD card.
-------------------------------------------------------------------------------
*/

uint8_t 
sd_receiveByteSPI (void);



/*
-------------------------------------------------------------------------------
|                             SEND COMMAND TO SD CARD
|                                        
| Description : Send a command and argument to the SD Card.
|
| Argument    : cmd    - 8-bit SD Card command to send to the SD Card. 
|             : arg    - 32-bit argument to be sent with the command.
-------------------------------------------------------------------------------
*/

void 
sd_sendCommand (uint8_t cmd, uint32_t arg);



/*
-------------------------------------------------------------------------------
|                             GET THE R1 RESPONSE
|                                        
| Description : Gets the R1 response after sending a cmd/arg to the SD card. 
|
| Return      : R1 Response Flags.
|               
| Note        : - Always call immediately after sd_sendCommand().
|               - Call sd_printR1(r1) to read the R1 response.
|               - R1_TIMEOUT is not a true SD card R1 response flag but is used
|                 here to indicate that the R1 response was not received. 
-------------------------------------------------------------------------------
*/

uint8_t 
sd_getR1 (void);



/*
-------------------------------------------------------------------------------
|                             PRINT THE R1 RESPONSE
|                                        
| Description : prints the R1 response flag(s) returned by sd_getR1(r1). 
|
| Argument    : r1    - byte. R1 response flag(s)
-------------------------------------------------------------------------------
*/

void 
sd_printR1 (uint8_t r1);



/*
-------------------------------------------------------------------------------
|                     PRINT THE INITIALIZATION RESPONSE FLAG
|                                        
| Description : Print the Initialization Error Flag portion of the response
|               returned by sd_spiModeInit(). 
|
| Argument    : err    - Initialization error response.
|
| Notes       : Though the Init Error Flags are only bits 8 to 19 of the Init
|               Error Response, it is valid to pass the entire Init Error 
|               Response to this function.
-------------------------------------------------------------------------------
*/

void 
sd_printInitError (uint32_t err);


#endif //SD_SPI_H