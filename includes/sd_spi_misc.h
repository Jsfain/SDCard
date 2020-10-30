/*
***********************************************************************************************************************
*                                                   AVR-SD CARD MODULE
*
* File    : SD_SPI_MISC.H
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
*
*
* DESCRIPTION:
* The file and it's header, are meant to be a catch-all for some specialized SD card functions that do not really fit
* into any of the other AVR-SD Card module files. These functions require SD_SPI_BASE.C/H and SD_SPI_DATA_ACCESS.C/H
* 
*
* FUNCTIONS:
*  (1) uint32_t SD_GetMemoryCapacitySC(void);
*  (2) uint32_t SD_GetMemoryCapacityHC(void);
*  (3) void     SD_FindNonZeroBlockNumbers(uint32_t startBlock, uint32_t endBlock);        
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



/*
***********************************************************************************************************************
 *                                                        FUNCTIONS
***********************************************************************************************************************
*/


// Calculate and return the memory capacity 
// of a standard capacity SD Card in Bytes.
uint32_t SD_GetMemoryCapacitySC(void);



// Calculate memory capacity of a high capacity SD Card
// from CSD and and return the memory capacity in Bytes.
uint32_t SD_GetMemoryCapacityHC(void);



// Prints the block number of all blocks that have any non-zero 
// bytes between startBlockAddress and endBlockAddress.
void SD_FindNonZeroBlockNumbers(uint32_t startBlock, uint32_t endBlock);


#endif // SD_SPI_MISC_H