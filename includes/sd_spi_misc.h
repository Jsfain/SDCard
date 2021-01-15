/*
*******************************************************************************
*                                  AVR-SD CARD MODULE
*
* File    : SD_SPI_MISC.H
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
* License : MIT
* Copyright (c) 2020
* 
*
* DESCRIPTION:
* This is meant to be a catch-all for some extra SD card functions that do not
* fit into any of the other AVR-SD Card module files. These functions require 
* SD_SPI_BASE.C/H and SD_SPI_DATA_ACCESS.C/H
*******************************************************************************
*/


#ifndef SD_SPI_MISC_H
#define SD_SPI_MISC_H





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
|                          GET SDSC MEMORY CAPACITY
|                                        
| Description : Calculates and returns the memory capacity of a standard 
|               capacity SD Card (SDSC)
|
| Return      : The memory capacity of the SD card in bytes.
-------------------------------------------------------------------------------
*/

uint32_t 
sd_getMemoryCapacitySDSC (void);



/*
-------------------------------------------------------------------------------
|                         GET SDHC / SDXC MEMORY CAPACITY
|                                        
| Description : Calculates and returns the memory capacity of a high/extended
|               capacity SD Card (SDHC/SDXC)
|
| Return      : The memory capacity of the SD card in bytes.
-------------------------------------------------------------------------------
*/

uint32_t 
sd_getMemoryCapacitySDHC (void);



/*
-------------------------------------------------------------------------------
|                         GET SDHC / SDXC MEMORY CAPACITY
|                                        
| Description : Search consecutive blocks over a specified range for those that
|               contain any non-zero values. The number/address of any blocks
|               found, with data will be printed to a screen. This function is
|               not fast, so searching over a large range of blocks can take a
|               while.
|
| Arguments   : startBlckAddr  - address of the first block to search.
|             : endBlckAddr    - address of the last block to search.
-------------------------------------------------------------------------------
*/

void 
sd_findNonZeroDataBlockNums (uint32_t startBlckAddr, uint32_t endBlckAddr);


/*
-------------------------------------------------------------------------------
|                           PRINT MULTIPLE BLOCKS
|                                        
| Description : Prints multiple blocks by calling the READ_MULTIPLE_BLOCKS SD
|               card command. The blocks that are read in will be printed by 
|               passing to sd_printSingleBlock().
|
| Arguments   : startBlckAddr - Address of the first block to be printed.
|             : numOfBlcks    - Number of blocks to be printed to the screen
|                               starting at startBlckAddr. 
|
| Return      : Error response. Upper byte holds a Read Block Error Flag. Lower
|               byte holds the R1 response. Pass to sd_printReadError(). If the
|               R1_ERROR flag is set, then the R1 response portion has an error 
|               which should then be read by sd_printR1().
-------------------------------------------------------------------------------
*/

uint16_t 
sd_printMultipleBlocks (uint32_t startBlckAddr, uint32_t numOfBlcks);



/*
-------------------------------------------------------------------------------
|                            WRITE TO MULTIPLE BLOCKS
|                                        
| Description : Write the contents of a byte array of length BLOCK_LEN to
|               multiple blocks of the SD card. The entire array data will be
|               copied to each block.
|
| Argument    : startBlckAddr - Address of first block to be written to.
|             : numOfBlcks    - Number of blocks to be written to.
|             : *dataArr      - Ptr to array of length BLOCK_LEN that holds
|                               the data that will be written to the SD Card.
|
| Return      : Error response. Upper byte holds a Write Block Error Flag.
|               Lower byte holds the R1 response. Pass to sd_printWriteError().
|               If the R1_ERROR flag is set, the R1 response portion has an 
|               error which should then be read by sd_printR1().
-------------------------------------------------------------------------------
*/

uint16_t 
sd_writeMultipleBlocks (uint32_t startBlckAddr, uint32_t numOfBlcks, 
                        uint8_t * dataArr);



/*
-------------------------------------------------------------------------------
|                      GET THE NUMBER OF WELL-WRITTEN BLOCKS
|                                        
| Description : This function sends the SD card command SEND_NUM_WR_BLOCKS. 
|               Call this after a failure on a WRITE_MULTIPLE_BLOCK command and
|               the Write Error Token is returned by the SD Card. This will 
|               provide the number of blocks that were successfully written
|               to before the error occurred.
|
| Argument    : *wellWrtnBlcks   - integer ptr to a value that will be updated 
|                                  by this function, and will specify the 
|                                  number of blocks successfully written to by
|                                  before the write error.
|
| Return      : Error response. Upper byte holds a Read Block Error Flag. The
|               lower byte holds the R1 response. Pass to sd_printWriteError().
|               If the R1_ERROR flag is set, the R1 response portion has an 
|               error which should then be read by sd_printR1().
-------------------------------------------------------------------------------
*/

uint16_t 
sd_getNumOfWellWrittenBlocks (uint32_t *wellWrittenBlocks);




#endif // SD_SPI_MISC_H