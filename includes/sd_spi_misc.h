/******************************************************************************
* File    : SD_SPI_MISC.H
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
* License : MIT
* Copyright (c) 2020
*
* Description:
* This is meant to be a catch-all for some misellaneous functions. Requires 
* SD_SPI_BASE and SD_SPI_RWE.
******************************************************************************/


#ifndef SD_SPI_MISC_H
#define SD_SPI_MISC_H





/******************************************************************************
 ******************************************************************************
 *                     
 *                               FUNCTIONS   
 *  
 ******************************************************************************
 *****************************************************************************/


/* ----------------------------------------------------------------------------
 *                                                          GET MEMORY CAPACITY
 * 
 * Functions to calculate and return the memory capacity of an SD Card in
 * bytes. Which one to use depends on the card type, i.e. SDSC or SDHC. This 
 * should be set in an instance of CTV while intializing the card.
 * ------------------------------------------------------------------------- */
uint32_t 
sd_getMemoryCapacitySDSC (void);

uint32_t 
sd_getMemoryCapacitySDHC (void);



/* ----------------------------------------------------------------------------
*                                                     FIND NON-ZERO DATA BLOCKS
*                                        
* DESCRIPTION: 
* Search consecutive blocks - from startBlckAddr to endBlckAddr - for thos that
* contain any non-zero values and print those block numbers/addresses to the
* screen. 
* 
* ARGUMENTS:
* 1) uint32_t startBlckAddr  - address of the first block to search.
* 2) uint32_t endBlckAddr    - address of the last block to search.
*
* NOTES:
* 1) This is mostly useful for finding which blocks may contain raw data.
* 2) This is not fast so suggest only search over a small range.
* -------------------------------------------------------------------------- */
void 
sd_findNonZeroDataBlockNums (uint32_t startBlckAddr, uint32_t endBlckAddr);



/* ----------------------------------------------------------------------------
 *                                                        PRINT MULTIPLE BLOCKS
 *           
 * DESCRIPTION:
 * Prints multiple blocks by calling the READ_MULTIPLE_BLOCK SD card command. 
 * Each block read in will be printed to the screen using sd_printSingleBlock()
 * function from SD_SPI_RWE.
 * 
 * ARGUMENTS:
 * 1) uint32_t startBlckAddr - Address of the first block to be printed.
 * 2) uint32_t numOfBlcks    - The number of blocks to be printed to the screen
 *                             starting at startBlckAddr. 
 * 
 * RETURN: 
 * uint16_t - Read Block Error (upper byte) and R1 Response (lower byte).
* -------------------------------------------------------------------------- */
uint16_t 
sd_printMultipleBlocks (uint32_t startBlckAddr, uint32_t numOfBlcks);



/* ----------------------------------------------------------------------------
 *                                                        WRITE MULTIPLE BLOCKS
 *           
 * DESCRIPTION:
 * Write the contents of a byte array of length BLOCK_LEN to multiple blocks of
 * the SD card. The entire array data will be copied to each block. This
 * function is not that useful, but used to test/demo the SD card command
 * WRITE_MULTIPLE_BLOCK.
 * 
 * ARGUMENTS:
 * 1) uint32_t startBlckAddr - Address of the first block to be written to.
 * 2) uint32_t numOfBlcks    - Number of blocks to be written to.
 * 3) uint8_t  *dataArr      - Ptr to an array holding the data contents that 
 *                             will be written to each of the specified SD 
 *                             card blocks. Array is of length BLOCK_LEN.
 * 
 * RETURN: 
 * uint16_t - Write Block Error (upper byte) and R1 Response (lower byte).
* -------------------------------------------------------------------------- */
uint16_t 
sd_writeMultipleBlocks (uint32_t startBlckAddr, uint32_t numOfBlcks, 
                        uint8_t * dataArr);



/* ----------------------------------------------------------------------------
 *                                        GET THE NUMBER OF WELL-WRITTEN BLOCKS
 *           
 * DESCRIPTION:
 * This function sends the SD card command SEND_NUM_WR_BLOCKS. This should be
 * called after a failure on a WRITE_MULTIPLE_BLOCK command and the Write 
 * Error Token is returned by the SD Card. This will provide the number of 
 * blocks that were successfully written before the error occurred.
 * 
 * ARGUMENTS:
 * uint32_t *wellWrtnBlcks - integer ptr to a value that will be updated by
 *                           by this function, and will specify the number of 
 *                           blocks successfully written to before the error.
 * 
 * RETURN: 
 * uint16_t - Read Block Error (upper byte) and R1 Response (lower byte).
* -------------------------------------------------------------------------- */
uint16_t 
sd_getNumOfWellWrittenBlocks (uint32_t *wellWrittenBlocks);




#endif // SD_SPI_MISC_H