# avr-SDCard
SD Card project - AVR ATmega1280 target 

# Purpose
Something to work on during quarantine.

# Description
This repository contains a project for raw data access of an SD Card using an ATmega1280 microcontroller via the SPI port.

# Details
Written in C and uses the avr-gcc.

What I call the base (or primary) SD functions are defined in SDBASE.C (declared in SDBASE.H). These are the functions that are required in order to interact with the SD card and a few print and get functions to simplify the code and print returned errors.  The functions include:
  1) sd_SPI_Mode_Init(): An SD initialization routine to intialize the SD card into SPI mode. This must be must be called first and complete successfully before any other interaction with the SD card will be successful/valid.
  2) sd_SendCommand() and sd_Response(): These functions are, as their names suggest, for sending a command to the SD card and returning the SD card's response.
  3) sd_CRC7(): calculates the CRC 7-bit value for a given command/arguement combination.  This function is called by sd_SendCommand which will send the CRC7 bit as part of the command, whether it is needed or not. By default CRC check is turned off for an SD Card in SPI mode, but it is required for the first few initialization commands.
  4)  sd_getR1(): routine to simplify getting the R1 SD card response to any command.
  5)  sd_printInitResponse() and sd_printR1(): Functions to print to screen, in human readable form, the SD Card R1 response or errors returned by the initialization routine.
  
Once the SD Card is initialized, all other interaction with the SD Card can be performed by calling the sd_Command and sd_Response functions.  The response must be interpreted and handled by the calling function as a single call to sd_Response will only return an 8-bit value, if the response if longer than 8-bits then sd_Response will need to be called repeatedly until the complete response has been read in.

The SD_MSG flag in SDBASE.H can be used for printing messages associated with functions in SDBASE.C.  This was helpful when writing the code so I left it in.  If you use this code, you may find it useful as well while troubleshooting.  Recommend setting this flag to 1 for normal operation (ERROR messages only).


An SDMISC.C is intended to be for any additional miscellaneous SD Card raw data access, calculation, and printing functions that I decide to create.

This is all still a work in progress.

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
