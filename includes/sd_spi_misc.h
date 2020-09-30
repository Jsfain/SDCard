/******************************************************************************
 * SD_SPI_MISC.H
 *  
 * TARGET
 * ATmega 1280
 *
 * DESCRIPTION
 * Uses SD_SPI_BASE.C(H) to implement specialized functions against an SD card
 * hosted on an AVR microcontroller and operating in SPI mode such as reading,
 * writing, erasing, and printing raw data blocks. The physical interaction
 * with the SD card is handled by functions defined in SD_SPI_BASE
 * 
 * FUNCTION LIST
 * 1) uint32_t  sd_GetMemoryCapacity(void)
 * 
 * Author: Joshua Fain
 * Date:   9/17/2020
 * ***************************************************************************/



#ifndef SD_SPI_MISC_H
#define SD_SPI_MISC_H



/******************************************************************************
 *                          FUNCTION DECLARATIONS
 *****************************************************************************/


/******************************************************************************
 * Function:    sd_getMemoryCapacity(void) 
 * Description: Calculates and returns the capacity of the SD card in bytes
 * Argument:    VOID
 * Returns:     uint32_t capcity of the SD card in bytes, if successful,
 *              and 1 if unsuccessful.
******************************************************************************/
uint32_t SD_GetMemoryCapacitySC(void);


uint32_t SD_GetMemoryCapacityHC(void);


/******************************************************************************
 * Function:    sd_SearchNonZeroBlocks(
 *                      uint32_t startBlock, 
 *                      uint32_t endBlock)
 * Description: Searches between a specified range of blocks for any blocks that
 *              have non-zero values and prints those block numbers to screen.
 * Argument:    1) uint32_t block number for the start block.
 *              2) uint32_t block number for the end block. 
 * Returns:     VOID
******************************************************************************/
void SD_FindNonZeroBlockNumbers(uint32_t startBlock, uint32_t endBlock);


#endif // SD_SPI_MISC_H