# avr-SDCard
SD Card project - AVR ATmega1280 target 

# Description
This repository contains a project for raw data access of an SD Card using an ATmega1280 microcontroller via the SPI port.

The primary SD functions are declared/defined in SDBASE.H/SDBASE.C, which includes:
  1)  A function to initialize a standard capacity SD Card in SPI mode.  This must complete successfully before any other SD Card interaction will be successful.
  2)  SD Command / Response functions to send commands to and return a response from the SD Card.
  3)  CRC7 calculation function to calculate the CRC7 value for a given command/arguement combination.  By default CRC check is turned off for an SD Card in SPI mode, 
      though it is required for the first few initialization commands.
  4)  Some print message functions which will print to screen a readable interpretation of the SD Card R1 response or error returned by the initialization routine.
  
Once the SD Card is initialized, all other interaction with the SD Card can be performed calling the Command and Response functions defined in SDBASE.C, though 
the response must be interpreted and handled by the calling function.

The SD_MSG flag in SDBASE.H can be used for printing messages associated with functions in SDBASE.C.  This was helpful when writing the code, and you may find it useful.
I would set this flag to 1 for normal operation (ERROR messages only).

SDMISC.H(C) is intended be for miscellaneous SD Card raw data access, calculation, and printing functions. WIP.

# Notes
* This code has only been tested using an ATmega1280 microcontroller, but assuming program data space and memory are sufficient, it is expected that it would easily be 
  portable to other comparable AVR microcontrollers once the USART and SPI port assignments are modified.
* Currently the code has only been tested with a 2GB SD card and thus only supports standard capacity SD cards (SDSC).

# Who can use
Anyone can use this and modify it to meet the needs of their system, but let me know if you use it or find it helpful.

# Requirements
[AVR Toolchain](https://github.com/osx-cross/homebrew-avr)

# Reference Documents:
1) Atmel ATmega640/V-1280/V-1281/V-2560/V-2561/V Datasheet
2) SD Specifications Part 1: Physical Layer Specification - Simplified Specification Version 7.10
