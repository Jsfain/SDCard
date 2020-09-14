# avr-SDCard
FAT Module - AVR ATmega1280 target 

# Purpose
Create a module for interacting with a physical FAT32 formatted volume.

# Details
Written in C
gcc version 5.4.0 (AVR_8_bit_GNU_Toolchain_3.6.2_503) 


This is all still a work in progress.

# Additional Notes / Limitations / Warnings
* Currently only read operations are allowed; there is no option to modify
   the contents of the volume. This means no files or directories nor any 
    of their property fields can be created or modified, nor can any FAT
     parameters be modified (i.e. boot sector/BPB, FSInfo, etc...)
* Currently only tested using a FAT32 formatted 2GB SD Card. IntendedThough the functions defined here are intended to work against any FAT32 formatted disk, 
    testing has only been done against a 2GB SD Card by including FAT32TOSD.C (FAT32TOSD.H) to
    interface between these FAT32 specific functions and the required disk specific functions.
    which implemenent the requirements mentioned above.
    Tries to mostly adhere to the standard, but still has some limitations.
        - Directories should not have extensions

# Who can use
Anyone can use this and modify it to meet the needs of their system, but let me know if you use it or find it helpful.

# Requirements
[AVR Toolchain](https://github.com/osx-cross/homebrew-avr)

* REQUIRES
* Driver to handle interaction with a physical disk. This module and the disk
* driver should interface by defining the following function:
*  
*  void FAT32_ReadSingleSector( uint32_t address, uint8_t *sectorByteArray)
*
* This functon should request the driver load the physical sector at 'address'
* into '*sectorByteArray'.

* In order to use these functions, the physical disk driver must be able to retrieve the contents
* of any single 512 byte sector in the formatted volume and then store this in a byte array of 
* length 512. Currently, only read operations are possible, so a driver function that can 
* implement the above requirements only needs to be mapped to the following function and either 
* included directly here or included via a new source/header file:


* "PUBLIC" FUNCTIONS
* (1) uint8_t   SetFatCurrentDirectory(
*                  FatCurrentDirectory *currentDirectory, 
*                  char *newDirectory)
* (2) uint8_t   PrintFatCurrentDirectoryContents(
*                  FatCurrentDirectory *currentDirectory, 
*                  uint8_t FLAG)
* (3) uint8_t   PrintFatFileContents(
*                  FatCurrentDirectory *currentDirectory,
*                  char *fileName)
* (4)  void     PrintFatError(uint8_t err)
* (5)  uint16_t GetFatBytsPerSec()
* (6)  uint8_t  GetFatSecPerClus()
* (7)  uint16_t GetFatRsvdSecCnt()
* (8)  uint8_t  GetFatNumFATs()
* (9)  uint32_t GetFatFATSz32()
* (10) uint32_t GetFatRootClus()

# Reference Documents:
1) Atmel ATmega640/V-1280/V-1281/V-2560/V-2561/V Datasheet
2) SD Specifications Part 1: Physical Layer Specification - Simplified Specification Version 7.10
