# SD Card Module
Used to execute the SPI mode SD card command set from an AVR host.

# Purpose
To establish a set of functions for access and control of an SD card by an AVR microcontroller. This module is intended to be used as a phyiscal disk layer operating underneath a file system module, however, it can simply be used by itself for raw data access (read/write/erase) and disk control as it is possible to execute any valid SD Card SPI Mode command from this module.

# Details
TARGET: AVR ATmega1280 microcontroller
LANGUAGE: C 
COMPILER: Successfully compiles with avr-gcc

# Overview
* The SD Card Module is itself modularized into base-level functions (SD_SPI_BASE) and data access functions (SD_DATA_ACCESS). 

* The base-level functions defined in SD_SPI_BASE.C are required by any implementation of this module.  Its primary tasks include:
  * Initializing the SD Card into SPI Mode.
  * Sending SPI-specific SD card commands/arguments to the SD card.
  * Receiving the SD card's returned bytes in response to any command sent.  
  * Additional helper functions. Their description is included in the "Detailed Description".


* The data access functions defined in (SD_DATA_ACCESS) require the base-level functions to handle specific data access routines, such as:
  * Reading and writing single data blocks.
  * Reading and writing mulitple data blocks.
  * Erasing data blocks.
  * Data print functions.
  * Additional miscellaneous functions. Their description is included in the "Detailed Description".

# Detailed Description
Base-Level Functions in SD_SPI.C:
  1) sd_SPI_Mode_Init(): An SD initialization routine to intialize the SD card into SPI mode. This    must be must be called first and complete successfully before any other interaction with the SD card will be successful/valid. Returns an initialization error code that also includes the most recent R1 response.  An initialization error code of 0 indicates the card has been successfully initialized. Any other error code indicates the initialization failed. Pass the error to sd_printInitResponse(err) to read the initialization error.
  2) sd_SendCommand(cmd, arg): Sends the command/argument/CRC7 combination to the SD card via the SPI port.  The CRC7 value is calculated and returned from sd_CRC7() which is called directly from the sd_SendCommand function.
  3) sd_Response(): Waits for, and returns the SD card's response for a given command. Only a single byte (8-bit) response will be returned each time this function is called so call this function must be called as many times as necessary to read in the full response to a command.

Helper Functions in SD_SPI.C:
  4) sd_CRC7(): calculates the CRC 7-bit value for a given command/arguement combination.  This function is called by sd_SendCommand which will send the CRC7 bit as part of the command, whether it is needed or not. By default CRC check is turned off for an SD Card in SPI mode, but it is required through command 8 (SEND_IF_COND) of the initializatin routine.
  5) sd_getR1(): routine to simplify getting the R1 SD card response to any command.
  6) sd_printR1(R1_response): interprets and prints to screen the R1 response returned for a give command in a human-readable form.
  6) sd_printInitResponse(): interprets and prints to screen the error code value returned by the intialization routine.  The initialization error code includes the most recent R1 response and thus sd_printR1() is also called from this function to print the R1 response portion of the initialization error.


Once the SD Card is initialized, all other host interaction with the SD Card can be performed by calling the sd_Command() and sd_Response() functions.  The response must be interpreted and handled by the calling function as a single call to sd_Response will only return an 8-bit value, if the response if longer than 8-bits then sd_Response will need to be called repeatedly until the complete response has been read in.

The SD_MSG flag in SD_SPI.H can be used for printing messages associated with functions in SD_SPI.C.  This was helpful when writing the code so I left it in.  If you use this code, you may find it useful as well while troubleshooting.  Recommend setting this flag to 1 for normal operation (ERROR messages only).

An SDMISC.C is intended to be for any additional miscellaneous SD Card raw data access, calculation, and printing functions that I decide to create.


# Additional Notes / Limitations / Warnings
* This code has only been tested using an ATmega1280 microcontroller, but assuming program data space and memory are sufficient, I would expect it to be easily ported to other comparable AVR microcontrollers once the USART and SPI port assignments are modified.
* Currently the code has only been tested with a 2GB SD card and thus only supports standard capacity SD cards (SDSC).

# Who can use
Anyone can use this and modify it to meet the needs of their system, but let me know if you use it or find it helpful.

# Requirements
[AVR Toolchain](https://github.com/osx-cross/homebrew-avr)

# Reference Documents:
1) Atmel ATmega640/V-1280/V-1281/V-2560/V-2561/V Datasheet
2) SD Specifications Part 1: Physical Layer Specification - Simplified Specification Version 7.10
