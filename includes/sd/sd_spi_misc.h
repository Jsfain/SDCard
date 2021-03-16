/*
 * File    : SD_SPI_MISC.H
 * Version : 0.0.0.1 
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : MIT
 * Copyright (c) 2020
 *
 * This is meant to be a catch-all for some misellaneous functions. Will 
 * require SD_SPI_BASE and SD_SPI_RWE.
 */

#ifndef SD_SPI_MISC_H
#define SD_SPI_MISC_H

#define FAILED_CAPACITY_CALC     1

#define NZDBN_PER_LINE           5

#define START_BLOCK_TKN_MBW      0xFC  // multi-block write start block token
#define STOP_TRANSMIT_TKN_MBW    0xFD  // stop multi-block write data TX


// memory capacity in bytes. See SD card standard for calc formula.
#define CAPACITY_CALC_SDHC(CSZ)   (((CSZ) + 1) * 512000)
/*
 ******************************************************************************
 *                                 FUNCTIONS   
 ******************************************************************************
 */

/* 
 * ----------------------------------------------------------------------------
 *                                                    FIND NON-ZERO DATA BLOCKS
 *                                        
 * Description : Gets the total byte capacity of the SD card and returns the 
 *               value. This function actually just operates to determine the
 *               card type (SDHC or SDSC) and uses this to call the correct 
 *               "private" function that will act to get the appropriate
 *               parameters from the card used and use these to calculate the 
 *               capacity. Which parameters and how the calculation is 
 *               performed is unique to the card type. 
 * 
 * Arguments   : ctv   - ptr to a CTV struct instance that. The value of the 
 *                       'type' member is used to determine which function to 
 *                       call to calculate the byte capacity.
 *
 * Returns     : The byte capacity of the SD card. If FAILED_CAPACITY_CALC (1)
 *               is returned instead, then the calc failed. This is a generic, 
 *               non-descriptive error and is used simply to indicate that an 
 *               issue was encountered during the process of getting the 
 *               capacity - this could include unknown card type, R1 error, 
 *               issue getting register contents, or something else.
 * ----------------------------------------------------------------------------
 */
uint32_t sd_GetCardByteCapacity(const CTV *ctv);

/* 
 * ----------------------------------------------------------------------------
 *                                                    FIND NON-ZERO DATA BLOCKS
 *                                        
 * Description : Search consecutive blocks, from startBlckAddr to endBlckAddr,
 *               inclusive, for those that contain any non-zero values and 
 *               print these block numbers / addresses to the screen.
 * 
 * Arguments   : startBlckAddr   - Address of the first block to search.
 *               endBlckAddr     - Address of the last block to search.
 *
 * Notes       : 1) Useful for finding which blocks may contain raw data.
 *               2) Not fast, so if using, suggest only searching over a small
 *                  range at a time.
 * ----------------------------------------------------------------------------
 */
void sd_FindNonZeroDataBlockNums(const uint32_t startBlckAddr, 
                                 const uint32_t endBlckAddr);

/* 
 * ----------------------------------------------------------------------------
 *                                                        PRINT MULTIPLE BLOCKS
 *           
 * Description : Prints the contents of multiple blocks by calling the 
 *               READ_MULTIPLE_BLOCK SD card command. Each block read in will
 *               be printed to the screen using sd_PrintSingleBlock() function
 *               from SD_SPI_RWE.
 * 
 * Arguments   : startBlckAddr   - Address of the first block to be printed.
 *               numOfBlcks      - The number of blocks to be printed to the 
 *                                 screen starting at startBlckAddr.
 * 
 * Returns     : Read Block Error (upper byte) and R1 Response (lower byte).
 * ----------------------------------------------------------------------------
 */
uint16_t sd_PrintMultipleBlocks(const uint32_t startBlckAddr, 
                                const uint32_t numOfBlcks);

/* 
 * ----------------------------------------------------------------------------
 *                                                        WRITE MULTIPLE BLOCKS
 *           
 * Description : Write the contents of a byte array of length BLOCK_LEN to 
 *               multiple blocks of the SD card. The entire array will be
 *               copied to each block. This function is mostly useful for 
 *               testing the WRITE_MULTIPLE_BLOCK SD card command.
 * 
 * Arguments   : startBlckAddr   - Address of the first block to be written.
 *               numOfBlcks      - Number of blocks to be written to.
 *               dataArr         - Pointer to an array holding the data 
 *                                 contents that will be written to each of 
 *                                 the specified SD card blocks. The array 
 *                                 must be of length BLOCK_LEN.
 * 
 * Returns     : Write Block Error (upper byte) and R1 Response (lower byte).
 * ----------------------------------------------------------------------------
 */
uint16_t sd_WriteMultipleBlocks(const uint32_t startBlckAddr, 
                                const uint32_t numOfBlcks, 
                                const uint8_t *dataArr);

/* 
 * ----------------------------------------------------------------------------
 *                                        GET THE NUMBER OF WELL-WRITTEN BLOCKS
 *           
 * Description : This function sends the SD card command SEND_NUM_WR_BLOCKS. 
 *               This should be called after a failure on a 
 *               WRITE_MULTIPLE_BLOCK command and the Write Error Token is 
 *               returned by the SD Card. This will provide the number of 
 *               blocks that were successfully written before the error 
 *               occurred.
 * 
 * Arguments   : wellWrtnBlcks     Pointer to a value that will be updated by
 *                                 by this function to specify the number of
 *                                 blocks successfully written to before the 
 *                                 error occurred.
 * 
 * Returns     : Read Block Error (upper byte) and R1 Response (lower byte).
 * ----------------------------------------------------------------------------
 */
uint16_t sd_GetNumOfWellWrittenBlocks(uint32_t* wellWrittenBlocks);

#endif // SD_SPI_MISC_H
