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


#endif // SD_SPI_MISC_H