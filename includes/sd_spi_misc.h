/*
***********************************************************************************************************************
*                                                   AVR-SDCARD MODULE
*
* File    : SD_SPI_DATA_MISC.H
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
*
*
* DESCRIPTION:
* Uses SD_SPI_BASE.C(H) to implement specialized functions against an SD card hosted on an AVR microcontroller and 
* operating in SPI mode such as reading, writing, erasing, and printing raw data blocks. The physical interaction with
* the SD card is handled by functions defined in SD_SPI_BASE.
* 
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