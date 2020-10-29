/*
***********************************************************************************************************************
*                                                   AVR-SDCARD MODULE
*
* File    : SD_SPI_BASE.H
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
*
*
* DESCRIPTION:
* Base-level SD card struct and macro definitions and function declarations that are used to handle the basic physical 
* interaction of the AVR-microcontroller with an SD card operating in SPI Mode.
*
*                                                 
* FUNCTIONS "PUBLIC":
*  (1) uint32_t SD_InitializeSPImode (CTV *ctv);
*  (2) void     SD_SendByteSPI (uint8_t byte);
*  (3) uint8_t  SD_ReceiveByteSPI (void);
*  (4) uvoid    SD_SendCommand(uint8_t cmd, uint32_t arg);
*  (5) uint8_t  SD_GetR1(void);
*  (6) void     SD_PrintR1(uint8_t r1);
*  (7) void     SD_PrintInitError(uint32_t err);
*
*
* STRUCTS USED (defined in SD_SPI_BASE.H)
*   typedef struct CardTypeVersion CTV;
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


#ifndef SD_SPI_H
#define SD_SPI_H



/*
***********************************************************************************************************************
 *                                                       MACROS
***********************************************************************************************************************
*/


// *********** Host Capcity Support

// 0 = host only supports SDSC. 
// 1 = host also supports SDHC or SDXC.
#define HCS 1



// *********** Block Length. 

// Currently only a value of 512 is supported.
#define BLOCK_LEN 512



// *********** CS Assert / De-assert
#define CS_LOW    SPI_PORT &= ~(1<<SS);  // Assert
#define CS_HIGH   SPI_PORT |= (1<<SS);   // Deassert



// ********** SD card commands

// The commands available for an
// SD Card operating in SPI mode
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

// With the exception of OUT_OF_IDLE state and R1_TIMEOUT flag,
// these are all possible flags that could be returned in the 
// SD Card's R1 response. R1_TIMEOUT can be set by the SD_GetR1()
// function if the SD card does not return the R1 response.
// These flags can be read using SD_PrintR1(uint8_t r1).
#define OUT_OF_IDLE             0x00   // No errors & out of idle
#define IN_IDLE_STATE           0x01
#define ERASE_RESET             0x02
#define ILLEGAL_COMMAND         0x04
#define COM_CRC_ERROR           0x08
#define ERASE_SEQUENCE_ERROR    0x10
#define ADDRESS_ERROR           0x20
#define PARAMETER_ERROR         0x40
#define R1_TIMEOUT              0x80



// *********** Initialization Error Flags

// These are flags are the possible return values of the SD_InitializationSPImode() 
// function. Note the lowest byte in this function's returned response will be 
// occupied by the most recent R1 response, therefore these flags begin at byte 2.
// These flags can be read using SD_PrintInitError(uint32_t err).
#define INIT_SUCCESS            0x00000
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
#define SDHC 1 // high capacity (or extended capacity SDXC)
#define SDSC 0 // standard capacity



/*
***********************************************************************************************************************
 *                                                       STRUCTS
***********************************************************************************************************************
*/


// ********** Card Type and Version 

// This struct will be used to hold the card's type and version. An instance of
// this struct should be passed to the initialization routine and its members
// will be set by that function. They should only be set by that function.
typedef struct CardTypeVersion {
    uint8_t version;
    uint8_t type;
} CTV;



/*
***********************************************************************************************************************
 *                                            "PUBLIC" FUNCTION DEFINITIONS
***********************************************************************************************************************
*/


/*
***********************************************************************************************************************
 *                                   INITIALIZE AN SD CARD INTO SPI MODE
 * 
 * Description : This function must be called first before implementing any other part of the AVR-SDCard module. This
 *               function will initialize the SD Card into SPI mode and set the CTV struct instance members to the 
 *               correct card type and version. 
 * 
 * Argument    : *ctv      Pointer to a CTV struct instance. This function will set the members of this instance.
 *                         The setting of this is critical so the other functions know how to handle block addressing.
 * 
 * Return      : Initialization Error Response      The initialization response will include an Initialization Error 
 *                                                  Flag in bits 8 to 19, and the most recent R1 Response Flag in bits
 *                                                  0 to 7. This can be read by passing the returned value to
 *                                                  SD_PrintInitError(uint32_t err) and SD_PrintR1(uint8_t r1).
***********************************************************************************************************************
*/

uint32_t SD_InitializeSPImode (CTV * ctv);



/*
***********************************************************************************************************************
 *                                          SEND BYTE TO THE SD CARD
 * 
 * Description : This function sends a single byte to the SD Card via the SPI port. This function along with
 *               SD_RecevieByteSPI() are the SPI port interfacing functions. Any interaction with the SD card directly
 *               uses these functions. 
 * 
 * Argument    : byte      The 8 bit packet that will be sent to the SD Card via the SPI port.
 * 
 * Note        : This function should be called as many times as required in order to send a complete data packet, 
 *               token, command, etc...
***********************************************************************************************************************
*/

void SD_SendByteSPI (uint8_t byte);



/*
***********************************************************************************************************************
 *                                        RECEIVE BYTE FROM THE SD CARD
 * 
 * Description : This function receives a single byte from the SD Card via the SPI port. This function along with
 *               SD_SendByteSPI() are the SPI port interfacing functions. Any interaction with the SD card directly
 *               uses these functions. 
 *
 * Argument    : void
 * 
 * Return      : 8 bit packet that will be sent by the SD card via the SPI port.
 * 
 * Note        : This function should be called as many times as required in order to receive a complete data packet, 
 *               token, error response, etc...
***********************************************************************************************************************
*/

uint8_t SD_ReceiveByteSPI (void);



/*
***********************************************************************************************************************
 *                                          SEND COMMAND TO SD CARD
 * 
 * Description : Send an available SPI mode command and argument to the SD Card. 
 * 
 * Argument    : cmd      - 8 bit unsigned integer specifying the command that the function will send to the SD Card. 
 *             : arg      - Argument that be sent with the command to the SD Card.
 **********************************************************************************************************************
*/

void SD_SendCommand (uint8_t cmd, uint32_t arg);



/*
***********************************************************************************************************************
 *                                          GET THE R1 RESPONSE
 * 
 * Description : Always call this function immediately after SD_SendCommand() to get the retured R1 response.
 * 
 * Argument    : void
 * 
 * Return      : R1 Response Flags
 * 
 * Note        : Call SD_PrintR1(uint8_t r1) to read the R1 response. 
***********************************************************************************************************************
*/

uint8_t SD_GetR1 (void);



/*
***********************************************************************************************************************
 *                                          PRINT THE R1 RESPONSE
 * 
 * Description : Call this function to print the R1 response returned by SD_GetR1().
 * 
 * Argument    : r1           R1 response returned by SD_GetR1();
***********************************************************************************************************************
*/

void SD_PrintR1 (uint8_t r1);



/*
***********************************************************************************************************************
 *                                      PRINT THE INITIALIZATION RESPONSE FLAG
 * 
 * Description : Call this function to print the Initialization Error Response portion of the value returned by the
 *               SD Card SPI mode initialization routine. This will only check bits 8 to 19 of the intializations
 *               returned value as the lowest byte (bits 0 to 8) are the most recent R1 response returned during 
 *               initialization, and should be read by SD_PrintR1().
 * 
 * Argument    : err          Initialization error response.
 ***********************************************************************************************************************
*/

void SD_PrintInitError (uint32_t err);


#endif //SD_SPI_H