/*
***********************************************************************************************************************
*                                                   AVR-SD CARD MODULE
*
* File    : SD_SPI_DATA_ACCESS.H
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
*
*
* DESCRIPTION:
* Implements some specialized raw data access functions against an SD card in SPI mode from an AVR microcontroller.
* Among the functions included here are single- and multi-block read, write, and erase as well as a few error printing
* functions. This file requires SD_SPI_BASE.C/H.
*
* FUNCTIONS:
*  (1)  uint16_t  SD_ReadSingleBlock (uint32_t blockAddress, uint8_t * blockArr);
*  (2)  void      SD_PrintSingleBlock (uint8_t * blockArr);
*  (3)  uint16_t  SD_WriteSingleBlock (uint32_t blockAddress, uint8_t * dataArr);
*  (4)  uint16_t  SD_EraseBlocks (uint32_t startBlockAddress, uint32_t endBlockAddress);
*  (5)  uint16_t  SD_PrintMultipleBlocks (uint32_t startBlockAddress, uint32_t numberOfBlocks);
*  (6)  uint16_t  SD_WriteMultipleBlocks (uint32_t blockAddress, uint32_t numberOfBlocks, uint8_t * dataArr);
*  (7)  uint16_t  SD_GetNumberOfWellWrittenBlocks (uint32_t * wellWrittenBlocks);
*  (8)  void      SD_PrintReadError (uint16_t err);
*  (9)  void      SD_PrintWriteError (uint16_t err);
*  (10) void      SD_PrintEraseError (uint16_t err);
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

#ifndef SD_SPI_DATA_ACCESS_H
#define SD_SPI_DATA_ACCESS_H



/*
***********************************************************************************************************************
 *                                                       MACROS
***********************************************************************************************************************
*/

 
// ********** R1 Error Flag 

// This flag is used to indicate the returned value has
// an error in the R1 response portion of the response.
#define R1_ERROR               0x8000



// ********** Read Block Error Flags

// Lower byte holds R1 portion of response. If the R1_ERROR flag
// is set, call SD_PrintR1() to read this R1 response.
#define START_TOKEN_TIMEOUT    0x0200
#define READ_SUCCESS           0x0400
//#define R1_ERROR             0x8000 //defined above



// *********** Write Block Error Flags

// Lower byte holds R1 portion of response. If the R1_ERROR flag
// is set, call SD_PrintR1() to read this R1 response.
#define DATA_ACCEPTED_TOKEN_RECEIVED    0x0100
#define CRC_ERROR_TOKEN_RECEIVED        0x0200
#define WRITE_ERROR_TOKEN_RECEIVED      0x0400
#define INVALID_DATA_RESPONSE           0x0800
#define DATA_RESPONSE_TIMEOUT           0x1000
#define CARD_BUSY_TIMEOUT               0x2000
//#define R1_ERROR                      0x8000 //defined above




// ********** Erase Error Flags

// Lower byte holds R1 portion of response. If the R1_ERROR flag
// is set, call SD_PrintR1() to read this R1 response.
#define ERASE_SUCCESSFUL           0x0000
#define SET_ERASE_START_ADDR_ERROR 0x0100
#define SET_ERASE_END_ADDR_ERROR   0x0200
#define ERASE_ERROR                0x0400
#define ERASE_BUSY_TIMEOUT         0x0800
//#define R1_ERROR                 0x8000 //defined above



/*
***********************************************************************************************************************
 *                                                        FUNCTIONS
***********************************************************************************************************************
*/

/*
***********************************************************************************************************************
 *                                               READ A SINGLE DATA BLOCK
 * 
 * Description : This function can be called to read the single data block from the SD Card at 'blockAddress' into the
 *               array pointed at by *blockArr
 * 
 * Arguments   : blockAddress    - Unsigned integer specifying the addressed location of the SD card block whose 
 *                                 contents will be printed to the screen. Addressing is determined by the card type:
 *                                 SDHC is Block Addressable, SDSC is Byte Addressable.
 *             : *blockArr       - Pointer to an array of length BLOCK_LEN (512 bytes). This function will load the
 *                                 contents of the SD card's block at 'blockAddress' into this array.
 * 
 * Return      : Error Response    The upper byte holds a Read Error Flag. The lower byte is the R1 response. Pass the
 *                                 returned value to SD_PrintReadError(err). If the R1_ERROR flag is set, then the R1
 *                                 response portion has an error, in which case this should be read by SD_PrintR1(err)
 *                                 from SD_SPI_BASE.
***********************************************************************************************************************
*/

uint16_t 
SD_ReadSingleBlock (uint32_t blockAddress, uint8_t *blockArr);



/*
***********************************************************************************************************************
 *                                               PRINT SINGLE BLOCK CONTENTS
 * 
 * Description : This function can be called to print the contents of a single block stored in an array. The contents
 *               of the block will be printed to the screen in rows of 16 bytes, columnized as: OFFSET | HEX | ASCII.
 *               The HEX and ASCII columns are different forms of the block's contents. The OFFSET column is the byte
 *               location of the first byte in each row relative to the location of the first byte in the block.     
 * 
 * Argument    : *blockArr       Pointer to a byte array of length BLOCK_LEN (512 bytes) that holds the data contents
 *                               of a single SD card block. This array should have previously been loaded by the
 *                               SD_ReadSingleBlock() function.
***********************************************************************************************************************
*/

void 
SD_PrintSingleBlock (uint8_t *blockArr);



/*
***********************************************************************************************************************
 *                                               WRITE TO A SINGLE BLOCK
 * 
 * Description : This function will write the contents of *dataArr to the block at 'blockAddress' on the SD card. The
 *               The data array should be of length BLOCK_LEN (512 bytes) and its entire contents will be written to
 *               the SD card block.
 * 
 * Arguments   : blockAddress   - Unsigned integer specifying the address of the block that will be written to. 
 *             : *dataArr       - Pointer to a byte array of length BLOCK_LEN (512 bytes) that holds the data contents
 *                                that will be written to block on the SD Card at blockAddress.
 * 
 * Return      : Error Response    The upper byte holds a Write Error Flag. The lower byte is the R1 response. Pass the
 *                                 returned value to SD_PrintWriteError(err). If the R1_ERROR flag is set, then the R1
 *                                 response portion has an error, in which case this should be read by SD_PrintR1(err)
 *                                 from SD_SPI_BASE.
***********************************************************************************************************************
*/


uint16_t 
SD_WriteSingleBlock (uint32_t blockAddress, uint8_t *dataArr);



/*
***********************************************************************************************************************
 *                                               ERASE BLOCKS
 * 
 * Description : This function will erase the blocks between (and including) startBlockAddress and endBlockAddress.
 * 
 * Argument    : startBlockAddress     - Address of the first block that will be erased.
 *             : endBlockAddress       - Address of the last block that will be erased.
 * 
 * Return      : Error Response   The upper byte holds an Erase Error Flag. The lower byte is the R1 response. Pass the
 *                                returned value to SD_PrintEraseError(err). If the R1_ERROR flag is set, then the R1
 *                                response portion has an error, in which case this should be read by SD_PrintR1(err)
 *                                from SD_SPI_BASE.
***********************************************************************************************************************
*/

uint16_t 
SD_EraseBlocks (uint32_t startBlockAddress, uint32_t endBlockAddress);



/*
***********************************************************************************************************************
 *                                               PRINT MULTIPLE BLOCKS
 * 
 * Description : This function will print multiple blocks by calling the SD card command READ_MULTIPLE_BLOCK. Every
 *               block that is read in will be printed to the screen by calling the SD_PrintSingleBlock() function.
 *               This function is not really that useful except as a demonstration of the READ_MULTIPLE_BLOCK command.
 * 
 * Argument    : startBlockAddress     - Address of the first block that will be printed to the screen.
 *             : numberOfBlocks        - The total number of consecutive blocks that will be printed to the screen.
 * 
 * Return      : Error Response    The upper byte holds a Read Error Flag. The lower byte is the R1 response. Pass the
 *                                 returned value to SD_PrintReadError(err). If the R1_ERROR flag is set, then the R1
 *                                 response portion has an error, in which case this should be read by SD_PrintR1(err)
 *                                 from SD_SPI_BASE.
***********************************************************************************************************************
*/

uint16_t 
SD_PrintMultipleBlocks (uint32_t startBlockAddress, uint32_t numberOfBlocks);



/*
***********************************************************************************************************************
 *                                               WRITE TO MULTIPLE BLOCKS
 * 
 * Description : This function will write the contents of a byte array of length BLOCK_LEN (512 bytes) to multiple 
 *               blocks of the SD card. The full array of data will be copied to each of the blocks. This function is 
 *               not really useful except to demontrate the WRITE_MULTIPLE_BLOCK SD card command.
 * 
 * Argument    : blockAddress          - Address of the first block that will be written.
 *             : numberOfBlocks        - The total number of consecutive blocks that will be written.
 *             : *dataArr              - Pointer to a byte array of length BLOCK_LEN (512 bytes) that holds the data
 *                                       contents that will be written to the SD Card.
 * 
 * Return      : Error Response    The upper byte holds a Write Error Flag. The lower byte is the R1 response. Pass the
 *                                 returned value to SD_PrintWriteError(err). If the R1_ERROR flag is set, then the R1
 *                                 response portion has an error, in which case this should be read by SD_PrintR1(err)
 *                                 from SD_SPI_BASE.
***********************************************************************************************************************
*/

uint16_t 
SD_WriteMultipleBlocks (uint32_t startBlockAddress, uint32_t numberOfBlocks, uint8_t * dataArr);



/*
***********************************************************************************************************************
 *                                        GET THE NUMBER OF WELL-WRITTEN BLOCKS
 * 
 * Description : This function can be called after a failure on a WRITE_MULTIPLE_BLOCK command and the Write Error 
 *               Token was returned by the SD Card. This function will issue the SD card command SEND_NUM_WR_BLOCKS
 *               which will return the number of blocks that were successfully written to.
 * 
 * Argument    : *wellWrittenBlocks       - Pointer to an integer that will be updated by this function to specify the 
 *                                          number of well-written blocks.
 * 
 * Return      : Error Response    The upper byte holds a Read Error Flag. The lower byte is the R1 response. Pass the
 *                                 returned value to SD_PrintReadError(err). If the R1_ERROR flag is set then the R1
 *                                 response portion has an error in which case this should be read by SD_PrintR1(err)
 *                                 from SD_SPI_BASE.
 ***********************************************************************************************************************
*/

uint16_t 
SD_GetNumberOfWellWrittenBlocks (uint32_t *wellWrittenBlocks);



/*
***********************************************************************************************************************
 *                                                 PRINT READ ERROR
 * 
 * Description : This function can be called to print the Read Error Flag returned by one of the read functions.
 * 
 * Argument    : err       Unsigned integer that should be a response returned by a read function.
***********************************************************************************************************************
*/

void 
SD_PrintReadError (uint16_t err);



/*
***********************************************************************************************************************
 *                                                PRINT WRITE ERROR
 * 
 * Description : This function can be called to print a Write Error Flag returned by one of the write functions.
 * 
 * Argument    : err       Unsigned integer that should be a response returned by a write function.
***********************************************************************************************************************
*/

void 
SD_PrintWriteError (uint16_t err);



/*
***********************************************************************************************************************
 *                                               PRINT ERASE ERROR
 * 
 * Description : This function can be called to print an Erase Error Flag returned by the erase function.
 * 
 * Argument    : err       Unsigned integer that should be a response returned by an erase function.
***********************************************************************************************************************
*/

void 
SD_PrintEraseError (uint16_t err);


#endif // SD_SPI_DATA_ACCESS_H
