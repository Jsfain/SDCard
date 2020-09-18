# FAT Filesystem Module
AVR Module for reading a FAT32 formatted disk/volume

# Purpose
To provide a module to read data/files from a FAT32 formatted disk/volume on an AVR platform.

# Details
TARGET: ATmega1280
LANGUAGE: C 
Successfully compiles with avr-gcc 
avr-gcc -v output -> gcc version 5.4.0 - AVR_8_bit_GNU_Toolchain_3.6.2_503

# Overview
This module provides a set of functions that can be used to read files from a FAT32 formatted disk. This module is intended to be independent of a physical disk and thus requires implementation with a disk driver/module to handle the task of retrieving data sectors requested by this module (see Disk Module requirements for specific requirements. 

This FAT module implements three primary operations for interacting with a FAT32 volume, which are separated into three primary functional task:

1) Directory Setter
2) Directory Reader
3) File Reader

The idea for their implementation is to initialize a current directory (CD) struct object whose members specify the 'current directory'.  A pointer to the CD object is then passed as the first argument to a function which will act upon it accordingly.  For instance, the "Directory Setter" function will search for a new directory (specified by the second argument) in the current directory and if this new directory is a child or parent of the current directory, then the members of the struct will be updated to point to the new directory.

A test.c file, which contains main() implements a 'Command-Line' like operation that can be used to change or print directories, or print a file.

See the Implementation section below for specifc instructions to implement the module and description of the functions.

# Disk Module Requirements
Before this module can be implemented a disk driver/module must be available to read raw data sectors from the physical disk. The requirements of the Physical Disk Driver are that it implement a function to read in single 512 byte raw data sectors. The FAT module should interface with the Disk Module by defining and making sure it is included in the FAT module: 

void FAT32_ReadSingleSector( uint32_t address, uint8_t *sectorByteArray)

This functon should operate by requesting the Disk Read function read in a 512 byte sector of raw data located at 'address' on the disk into a 512 byte array pointed at by *sectorByteArray.  The sector address must be the byte address of the first byte of the 512 byte sector, not the sector number.  So if reading sector number 1000 the address 512 * 1000 = 512,000 (assuming the phyiscal sectors are 512 bytes which is the only thing currently supported by the FAT module).

EXAMPLE:

void fat_ReadSingleSector( uint32_t address, uint8_t *sectorByteArray)
{
    DataBlock db;
    db = sd_ReadSingleDataBlock(address);
    for(int k = 0; k < 512; k++) { sectorByteArray[k] = db.data[k]; }
};

The file FATtoSD.C defines the required function above to interface with an SD Card by calling a function called sd_ReadSingleSector(address) which passes the sector bytes into a db struct object with a member (uint8_t data[512]) and then passes this to the sectorByteArray.


Current directory struct: 
typedef FatCurrentDirectory
Five Members:
    * char longName[256] - array to hold the long name string of the current directory. Max 255 characters.
    * char longParentPath[256] - array to hold the long name path to the current directory's parent directory.
    * char shortName[9] - array to hold the short name string of the current directory. Only holds the name portion of the 8.3 standard. Extensions in directory names has not been tested.
    * char shortParentPath[256] - array to hold the short name path to the current directory's parent directory.
    * uint32_t FATFirstCluster - the index of the first cluster for the current directory in the FAT.  This value must be correctly set to point to a valid directory's first cluster.


# Primary Functions and how to use them description:
Section to describe how to use the primary functions:

1) A Directory Setter - SetFatCurrentDirectory(FatCurrentDirectory *currentDirectory, char *newDirectory)

2) A Directory Reader - PrintFatCurrentDirectoryContents(FatCurrentDirectory *currentDirectory, uint8_t FLAG)
3) A File Reader      - PrintFatFileContents(FatCurrentDirectory *currentDirectory, char *fileName)


# Known Limitations / Warnings
* Currently only read operations are allowed; there is no option to modify the contents of the volume. This means no files or directories nor any of their property fields can be created or modified, nor can any FAT parameters be modified (i.e. boot sector/BPB, FAT, FSInfo, etc...)
* Though the module is designed to operate independent of the physical disk, this module has only been tested using a FAT32 formatted 2GB mini SD Card using the SDCard module and implementing the FAT32TOSD interface (which is included with this module).
* There is no guarantee it 100% adhere's to the FAT standard.
    * Directories should not have extensions. 

# Who can use
Anyone can use this and modify it to meet the needs of their system, but let me know if you use it or find it helpful.

# Requirements
[AVR Toolchain](https://github.com/osx-cross/homebrew-avr)

# List of Functions
* uint8_t   SetFatCurrentDirectory(FatCurrentDirectory *currentDirectory, char *newDirectory)
* uint8_t   PrintFatCurrentDirectoryContents(FatCurrentDirectory *currentDirectory, uint8_t FLAG)
* uint8_t   PrintFatFileContents(FatCurrentDirectory *currentDirectory, char *fileName)
* void      PrintFatError(uint8_t err)
* uint16_t  GetFatBytsPerSec()
* uint8_t   GetFatSecPerClus()
* uint16_t  GetFatRsvdSecCnt()
* uint8_t   GetFatNumFATs()
* uint32_t  GetFatFATSz32()
* uint32_t  GetFatRootClus()

# Reference Documents:
1) Atmel ATmega640/V-1280/V-1281/V-2560/V-2561/V Datasheet
2) SD Specifications Part 1: Physical Layer Specification - Simplified Specification Version 7.10
