/*
*******************************************************************************
*                                  AVR-SD CARD MODULE
*
* File    : SD_SPI_DATA_ACCESS.H
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
* License : MIT
* Copyright (c) 2020
* 
*
* DESCRIPTION:
* Interface for some specialized raw data access functions against an SD card 
* operating in SPI mode using an AVR microcontroller, e.g. single- and multi-
* block read, write, and erase as well as a few error printing functions. 
* This interface requires SD_SPI_BASE.C/H to operate.
*******************************************************************************
*/


#ifndef SD_SPI_DATA_ACCESS_H
#define SD_SPI_DATA_ACCESS_H





/*
*******************************************************************************
*******************************************************************************
 *                     
 *                               MACROS   
 *  
*******************************************************************************
*******************************************************************************
*/
 
// ********** R1 Error Flag 

// This flag is used to indicate the returned value has
// an error in the R1 response portion of a response.
#define R1_ERROR               0x8000



// ********** Read Block Error Flags

// Lower byte holds R1 portion of response. If the R1_ERROR flag
// is set, call sd_printR1() to read this R1 response.
#define START_TOKEN_TIMEOUT    0x0200
#define READ_SUCCESS           0x0400
//#define R1_ERROR             0x8000 //defined above



// *********** Write Block Error Flags

// Lower byte holds R1 portion of response. If the R1_ERROR flag
// is set, call sd_printR1() to read this R1 response.
#define DATA_ACCEPTED_TOKEN_RECEIVED    0x0100
#define CRC_ERROR_TOKEN_RECEIVED        0x0200
#define WRITE_ERROR_TOKEN_RECEIVED      0x0400
#define INVALID_DATA_RESPONSE           0x0800
#define DATA_RESPONSE_TIMEOUT           0x1000
#define CARD_BUSY_TIMEOUT               0x2000
//#define R1_ERROR                      0x8000 //defined above




// ********** Erase Error Flags

// Lower byte holds R1 portion of response. If the R1_ERROR flag
// is set, call sd_printR1() to read this R1 response.
#define ERASE_SUCCESSFUL           0x0000
#define SET_ERASE_START_ADDR_ERROR 0x0100
#define SET_ERASE_END_ADDR_ERROR   0x0200
#define ERASE_ERROR                0x0400
#define ERASE_BUSY_TIMEOUT         0x0800
//#define R1_ERROR                 0x8000 //defined above





/*
*******************************************************************************
*******************************************************************************
 *                     
 *                               FUNCTIONS   
 *  
*******************************************************************************
*******************************************************************************
*/

/*
-------------------------------------------------------------------------------
|                          READ A SINGLE DATA BLOCK
|                                        
| Description : Reads the single data block from the SD Card at blckAddr into 
|               the array pointed at by *blckArr.
|
| Arguments   : blckAddr    - address of the block on the SD card block whose 
|                             contents will be printed to the screen.
|             : *blckArr    - ptr to an array that will be loaded with the
|                             contents of the SD card's block at 'blckAddr'.
|                             Array should be of length BLOCK_LEN.
|
| Return      : Error response. The upper byte holds the Read Block Error Flag.
|               The lower byte holds the R1 response. Pass to 
|               sd_printReadError(). If R1_ERROR flag is set, then the R1 
|               response portion has an error. This should be read by 
|               sd_printR1().
|
| Notes       : Addressing is determined by the card type. SDHC is Block 
|               addressable, SDSC is byte addressable. This should be known 
|               before passing the address.
-------------------------------------------------------------------------------
*/

uint16_t 
sd_readSingleBlock (uint32_t blckAddr, uint8_t *blckArr);



/*
-------------------------------------------------------------------------------
|                                PRINT SINGLE BLOCK
|                                        
| Description : Prints the contents of a single block stored in an array to a
|               screen. The block's contents will be printed to the screen in
|               rows of 16 bytes, columnized as (Addr)OFFSET | HEX | ASCII.
|
| Argument    : *blckArr    - ptr to an array holding the contents of the block
|                             to be printed. Array should be length BLOCK_LEN.
|                             Array should be loaded by sd_readSinbleBlock().
-------------------------------------------------------------------------------
*/

void 
sd_printSingleBlock (uint8_t *blckArr);



/*
-------------------------------------------------------------------------------
|                               WRITE SINGLE BLOCK
|                                        
| Description : Writes contents of array *dataArr to the block at 'blckAddr'
|               on the SD card. The data array should be of length BLOCK_LEN.
|               Its entire contents will be written to the SD card block.
|
| Arguments   : blckAddr   - address of the block that will be written to. 
|             : *dataArr   - ptr to an array that holds the data contents
|                            that will be written to the block on the SD Card.
|
| Return      : Error response. Upper byte holds the Write Block Error Flag.
|               Lower byte holds the R1 response. Pass to sd_printReadError().
|               If R1_ERROR flag is set, then the R1 response portion has an 
|               error which should be read by sd_printR1().
-------------------------------------------------------------------------------
*/


uint16_t 
sd_writeSingleBlock (uint32_t blckAddr, uint8_t *dataArr);



/*
-------------------------------------------------------------------------------
|                               ERASE BLOCKS
|                                        
| Description : This function will erase the blocks between (and including) 
|               startBlckAddr and endBlckAddr.
|
| Arguments   : startBlckAddr - Address of the first block that will be erased.
 *            : endBlckAddr   - Address of the last block that will be erased.
|
| Return      : Error response. Upper byte holds the Erase Error Flag. Lower
|               byte holds the R1 response. Pass to sd_printEraseError().
|               If R1_ERROR flag is set, then the R1 response portion has an 
|               error which should be read by sd_printR1().
-------------------------------------------------------------------------------
*/

uint16_t 
sd_eraseBlocks (uint32_t startBlckAddr, uint32_t endBlckAddr);



/*
-------------------------------------------------------------------------------
|                                PRINT READ ERROR
|                                        
| Description : Print Read Error Flag returned by a SD card read function.
|
| Argument    : err  - byte holding a Read Error Response.
-------------------------------------------------------------------------------
*/

void 
sd_printReadError (uint16_t err);



/*
-------------------------------------------------------------------------------
|                                PRINT WRITE ERROR
|                                        
| Description : Print Write Error Flag returned by a SD card write function.
|
| Argument    : err  - byte holding a Read Error Response.
-------------------------------------------------------------------------------
*/

void 
sd_printWriteError (uint16_t err);



/*
-------------------------------------------------------------------------------
|                                PRINT ERASE ERROR
|                                        
| Description : Print Erase Error Flag returned by an SD card erase function.
|
| Argument    : err  - byte holding a Read Error Response.
-------------------------------------------------------------------------------
*/

void 
sd_printEraseError (uint16_t err);


#endif // SD_SPI_DATA_ACCESS_H
