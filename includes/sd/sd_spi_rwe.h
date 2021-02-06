/*
 * File    : SD_SPI_RWE.H
 * Version : 0.0.0.1 
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : MIT
 * Copyright (c) 2020-2021
 * 
 * Interface for some functions that implement SD Card single-block read/print, 
 * write, and multi-block erase by calling the necessary SD commands provided
 * in SD_SPI_CMDS.H. These functions require SD_SPI_BASE.H/C.
 */

#ifndef SD_SPI_RWE_H
#define SD_SPI_RWE_H

/*
 ******************************************************************************
 *                                  MACROS   
 ******************************************************************************
 */
 
/* 
 * ----------------------------------------------------------------------------
 *                                                                R1 ERROR FLAG
 *
 * Description : Flag used to indicate a functions's returned value contains an
 *               error in the R1 response portion.
 * ----------------------------------------------------------------------------
 */
#define R1_ERROR                        0x8000


/* 
 * ----------------------------------------------------------------------------
 *                                                       READ BLOCK ERROR FLAGS
 *
 * Description : Flags returned by READ block functions, 
 *               e.g. sd_readSingleBlock().
 * 
 * Notes       : The lower byte is reserved for the R1 Response Flags, 
 *               see SD_SPI_BASE.H. 
 * ----------------------------------------------------------------------------
 */
#define START_TOKEN_TIMEOUT             0x0200
#define READ_SUCCESS                    0x0400


/* 
 * ----------------------------------------------------------------------------
 *                                                      WRITE BLOCK ERROR FLAGS
 *
 * Description : Flags returned by WRITE block functions, 
 *               e.g. sd_writeSingleBlock().
 * 
 * Notes       : The lower byte is reserved for the R1 Response Flags,
 *               see SD_SPI_BASE.H. 
 * ----------------------------------------------------------------------------
 */
#define DATA_ACCEPTED_TOKEN_RECEIVED    0x0100
#define CRC_ERROR_TOKEN_RECEIVED        0x0200
#define WRITE_ERROR_TOKEN_RECEIVED      0x0400
#define INVALID_DATA_RESPONSE           0x0800
#define DATA_RESPONSE_TIMEOUT           0x1000
#define CARD_BUSY_TIMEOUT               0x2000



/* 
 * ----------------------------------------------------------------------------
 *                                                      ERASE BLOCK ERROR FLAGS
 *
 * Description : Flags returned by ERASE block functions, 
 *               e.g. sd_eraseSingleBlock().
 * 
 * Notes       : The lower byte is reserved for the R1 Response Flags, 
 *               see SD_SPI_BASE.H. 
 * ----------------------------------------------------------------------------
 */
#define ERASE_SUCCESSFUL                0x0000
#define SET_ERASE_START_ADDR_ERROR      0x0100
#define SET_ERASE_END_ADDR_ERROR        0x0200
#define ERASE_ERROR                     0x0400
#define ERASE_BUSY_TIMEOUT              0x0800


/*
 ******************************************************************************
 *                               FUNCTIONS   
 ******************************************************************************
 */


/*
 * For the following read, write, and erase block functions, the returned error
 * response values can be read by their corresponding print error function. For
 * example, the returned value of sd_readSingleBlock() can be read by passing
 * it to sd_printReadError(). These print functions will read the upper byte of
 * of the returned error response. If in the error response the R1_ERROR flag 
 * is set in the upper byte, then the lower byte (i.e. the R1 Response portion
 * of the error response) contains at least one flag that has been set which 
 * should then be read by passing it to sd_printR1() in SD_SPI_BASE. 
 */ 


/*
 * ----------------------------------------------------------------------------
 *                                                            READ SINGLE BLOCK
 * 
 * Description : Reads a single data block from the SD card into an array.     
 * 
 * Arguments   : blckAddr     address of the data block on the SD card that    
 *                            will be read into the array.
 * 
 *               blckArr      pointer to the array to be loaded with the       
 *                            contents of the data block at blckAddr. Must be  
 *                            length BLOCK_LEN.
 * 
 * Returns     : Read Block Error (upper byte) and R1 Response (lower byte).   
 * ----------------------------------------------------------------------------
 */

uint16_t sd_readSingleBlock(uint32_t blckAddr, uint8_t* blckArr);


/*
 * ----------------------------------------------------------------------------
 *                                                           PRINT SINGLE BLOCK
 * 
 * Description : Prints the contents of a single SD card data block, previously 
 *               loaded into an array by sd_readSingleBlock(), to the screen. 
 *               The block's contents will be printed to the screen in rows of
 *               16 bytes, columnized as (Addr)OFFSET | HEX | ASCII.
 * 
 * Arguments   : blckArr     pointer to an array holding the contents of the 
 *                           block to be printed to the screen. Must be of 
 *                           length BLOCK_LEN.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */

void sd_printSingleBlock(uint8_t* blckArr);


/*
 * ----------------------------------------------------------------------------
 *                                                           WRITE SINGLE BLOCK
 * 
 * Description : Writes the values in an array to a single SD card data block.
 * 
 * Arguments   : blckAddr     address of the data block on the SD card that 
 *                            will be written to.
 *       
 *               dataArr      pointer to an array that holds the data contents
 *                            that will be written to the block at blckAddr on
 *                            the SD card. Must be of length BLOCK_LEN.
 * 
 * Returns     : Write Block Error (upper byte) and R1 Response (lower byte).
 * ----------------------------------------------------------------------------
 */

uint16_t sd_writeSingleBlock(uint32_t blckAddr, uint8_t* dataArr);


/*
 * ----------------------------------------------------------------------------
 *                                                                 ERASE BLOCKS
 * 
 * Description : Erases the blocks between (and including) the startBlckAddr 
 *               and endBlckAddr.
 * 
 * Arguments   : startBlckAddr     address of the first block to be erased.
 * 
 *               endBlckAddr       address of the last block to be erased.
 * 
 * Returns     : Erase Block Error (upper byte) and R1 Response (lower byte).
 * ----------------------------------------------------------------------------
 */

uint16_t sd_eraseBlocks(uint32_t startBlckAddr, uint32_t endBlckAddr);


/*
 * If either of the three print error functions show that the R1_ERROR flag was
 * set in the error response that was passed to it, then the error response
 * should then be passed to sd_printR1() from SD_SPI_BASE.H/C to read the 
 * R1 Error.
 */

/*
 * ----------------------------------------------------------------------------
 *                                                             PRINT READ ERROR
 * 
 * Description : Print Read Error Flag returned by a SD card read function.  
 * 
 * Arguments   : err     Read Error Response.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */

void sd_printReadError(uint16_t err);


/*
 * ----------------------------------------------------------------------------
 *                                                            PRINT WRITE ERROR
 * 
 * Description : Print Write Error Flag returned by a SD card read function.  
 * 
 * Arguments   : err     Write Error Response.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */

void sd_printWriteError(uint16_t err);


/*
 * ----------------------------------------------------------------------------
 *                                                            PRINT ERASE ERROR
 * 
 * Description : Print Erase Error Flag returned by a SD card read function.  
 * 
 * Arguments   : err     Erase Error Response.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */

void sd_printEraseError(uint16_t err);

#endif // SD_SPI_RWE_H
