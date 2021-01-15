/******************************************************************************
 * File    : SD_SPI_RWE.H
 * Version : 0.0.0.1 
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : MIT
 * Copyright (c) 2020-2021
 * 
 * Description:
 * Interface for some functions that implement SD Card single-block read/print, 
 * write, and multi-block erase by calling the necessary SD commands provided
 * in SD_SPI_CMDS.H. These functions require SD_SPI_BASE.H/C.
 * ***************************************************************************/



#ifndef SD_SPI_RWE_H
#define SD_SPI_RWE_H





/******************************************************************************
 ******************************************************************************
 *                     
 *                               MACROS   
 *  
 ******************************************************************************
 *****************************************************************************/
 

/* ----------------------------------------------------------------------------
 *                                                                R1 ERROR FLAG
 *
 * DESCRIPTION:
 * Flag used to indicate a functions's returned value contains an error in the 
 * R1 response portion.
 --------------------------------------------------------------------------- */
#define R1_ERROR                        0x8000



/* ----------------------------------------------------------------------------
 *                                                       READ BLOCK ERROR FLAGS
 *
 * DESCRIPTION:
 * Flags returned by READ block functions, e.g. sd_readSingleBlock().
 * 
 * NOTES:
 * The lower byte is reserved for the R1 Response Flags, see SD_SPI_BASE.H. 
 --------------------------------------------------------------------------- */
#define START_TOKEN_TIMEOUT             0x0200
#define READ_SUCCESS                    0x0400



/* ----------------------------------------------------------------------------
 *                                                      WRITE BLOCK ERROR FLAGS
 *
 * DESCRIPTION:
 * Flags returned by WRITE block functions, e.g. sd_writeSingleBlock().
 * 
 * NOTES:
 * The lower byte is reserved for the R1 Response Flags, see SD_SPI_BASE.H. 
 --------------------------------------------------------------------------- */
#define DATA_ACCEPTED_TOKEN_RECEIVED    0x0100
#define CRC_ERROR_TOKEN_RECEIVED        0x0200
#define WRITE_ERROR_TOKEN_RECEIVED      0x0400
#define INVALID_DATA_RESPONSE           0x0800
#define DATA_RESPONSE_TIMEOUT           0x1000
#define CARD_BUSY_TIMEOUT               0x2000




/* ----------------------------------------------------------------------------
 *                                                      ERASE BLOCK ERROR FLAGS
 *
 * DESCRIPTION:
 * Flags returned by ERASE block functions, e.g. sd_eraseSingleBlock().
 * 
 * NOTES:
 * The lower byte is reserved for the R1 Response Flags, see SD_SPI_BASE.H. 
 --------------------------------------------------------------------------- */
#define ERASE_SUCCESSFUL                0x0000
#define SET_ERASE_START_ADDR_ERROR      0x0100
#define SET_ERASE_END_ADDR_ERROR        0x0200
#define ERASE_ERROR                     0x0400
#define ERASE_BUSY_TIMEOUT              0x0800






/******************************************************************************
 ******************************************************************************
 *                     
 *                               FUNCTIONS   
 *  
 ******************************************************************************
 *****************************************************************************/

/*
 * For the following block read, write, and erase block, the returned error
 * response values can be read by their corresponding print error function. For
 * example, the returned value of sd_readSingleBlock() can be read by passing
 * it to sd_printReadError(). These print functions will read the upper byte of
 * of the error response. If in the error response the R1_ERROR flag is set in
 * the upper byte, then the lower byte (i.e. the R1 Response portion of the
 * error response) contains at least one flag that has been set which should 
 * then be read by passing the error response to sd_printR1() in SD_SPI_BASE. 
 */ 



/*-----------------------------------------------------------------------------
 *                                                            READ SINGLE BLOCK
 * 
 * DESCRIPTION: 
 * Reads a single data block from the SD card into an array.   
 * 
 * ARGUMENTS:
 * 1) uint32_t blckAddr - Address of the data block on the SD card that will be
 *                        read into the array.
 * 2) uint8_t  *blckArr - Ptr to an array to be loaded with the contents of the 
 *                        data block at blckAddr. Must be of length BLOCK_LEN.
 * 
 * RETURN: 
 * uint16_t - Read Block Error (upper byte) and R1 Response (lower byte).
 * ------------------------------------------------------------------------- */
uint16_t 
sd_readSingleBlock (uint32_t blckAddr, uint8_t *blckArr);




/*-----------------------------------------------------------------------------
 *                                                           PRINT SINGLE BLOCK
 * 
 * DESCRIPTION: 
 * Prints the contents of a single SD card data block, that's previously been 
 * loaded into an array, to the screen. The block's contents will be printed to
 * the screen in rows of 16 bytes, columnized as (Addr)OFFSET | HEX | ASCII.
 * 
 * ARGUMENTS: 
 * uint8_t  *blckArr - Ptr to an array holding the contents of the block to be 
 *                     printed to the screen. Must be of length BLOCK_LEN.
 * 
 * RETURN:
 * void
 * ------------------------------------------------------------------------- */
void 
sd_printSingleBlock (uint8_t *blckArr);



/*-----------------------------------------------------------------------------
 *                                                           WRITE SINGLE BLOCK
 * 
 * DESCRIPTION: 
 * Writes the values in an array to a single SD card data block.   
 * 
 * ARGUMENTS:
 * 1) uint32_t blckAddr - Address of the data block on the SD card that will be
 *                        written to.
 * 2) uint8_t  *dataArr - Ptr to an array that holds the data contents that
 *                        will be written to the block at blckAddr on the SD 
 *                        Card. Must be of length BLOCK_LEN.
 * 
 * RETURN: 
 * uint16_t - Write Block Error (upper byte) and R1 Response (lower byte).
 * ------------------------------------------------------------------------- */
uint16_t 
sd_writeSingleBlock (uint32_t blckAddr, uint8_t *dataArr);



/*-----------------------------------------------------------------------------
 *                                                                 ERASE BLOCKS
 * 
 * DESCRIPTION: 
 * Erases the blocks between (and including) the startBlckAddr and endBlckAddr.   
 * 
 * ARGUMENTS:
 * 1) uint32_t startBlckAddr - Address of the first block that will be erased.
 * 2) uint32_t endBlckAddr   - Address of the last block that will be erased.
 * 
 * RETURN: 
 * uint16_t - Erase Block Error (upper byte) and R1 Response (lower byte).
 * ------------------------------------------------------------------------- */
uint16_t 
sd_eraseBlocks (uint32_t startBlckAddr, uint32_t endBlckAddr);





/*
 * If either of the three print error functions show that the R1_ERROR flag was
 * set in the error response that was passed to it, then the error response
 * should then be passed to sd_printR1() from SD_SPI_BASE.H/C to read the 
 * R1 Error.
 */



/*-----------------------------------------------------------------------------
 *                                                             PRINT READ ERROR
 * 
 * DESCRIPTION: 
 * Print Read Error Flag returned by a SD card read function.  
 * 
 * ARGUMENTS:
 * uint16_t err - Read Error Response.
 * 
 * RETURN: 
 * void
 * ------------------------------------------------------------------------- */
void 
sd_printReadError (uint16_t err);


/*-----------------------------------------------------------------------------
 *                                                            PRINT WRITE ERROR
 * 
 * DESCRIPTION: 
 * Print Write Error Flag returned by a SD card read function.  
 * 
 * ARGUMENTS:
 * uint16_t err - Write Error Response.
 * 
 * RETURN: 
 * void
 * ------------------------------------------------------------------------- */
void 
sd_printWriteError (uint16_t err);



/*-----------------------------------------------------------------------------
 *                                                            PRINT ERASE ERROR
 * 
 * DESCRIPTION: 
 * Print Erase Error Flag returned by a SD card read function.  
 * 
 * ARGUMENTS:
 * uint16_t err - Erase Error Response.
 * 
 * RETURN: 
 * void
 * ------------------------------------------------------------------------- */
void 
sd_printEraseError (uint16_t err);



#endif // SD_SPI_RWE_H
