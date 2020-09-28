# AVR-SD Card Module
For executing SPI mode SD card commands running on an AVR host.


## Purpose
To establish a set of functions for SD card access and control in SPI mode from an AVR microcontroller. The module is intended for standalone raw data access, but may also be used for other puposes, e.g. a physical disk layer for a file system.


## Technologies
* LANGUAGE - C
* TARGET - ATmega1280 - other AVR targets may be used with modification of ports assignments, assuming memory is sufficient.
* AVR-Toolchain 9.3.0 - includes AVR-GCC and AVRDUDE. 
* AVR-GCC 9.3.0 - required to compile and build the module. Included in the AVR-Toolchain.
* AVRDUDE 6.3 - required to download the build to the AVR. Included in the AVR-Toolchain.

### Comments
* This project was originally built on a MAC with the [AVR-Toolchain](https://github.com/osx-cross/homebrew-avr) available from Homebrew.  For Windows, Atmel Studio should come with a version of the toolchain, but I have no experience with this version.


## Overview
The module is composed of three source/header files:

1) SD_SPI_BASE.C(H)
    * Only this source/header is required.
    * Includes the basic functions for direct interaction with the SD card e.g. a routine for initializing the SD card into SPI mode, a function for sending commands to the SD card, and one for receiving responses in byte-sized packets from the SD card. SD_SPI_BASE also includes some error printing functions and some additional helper functions.

2) SD_SPI_DATA_ACCESS.C(H)
    * Includes some specific functions for handling data access such as reading, writing, and erasing data blocks as well as some error print functions.. 
    * Requires SD_SPI_BASE to handle sending commands and receiving the responses.

3) SD_SPI_MISC.C(H)
    * Intended to hold some miscellaneous functions. 
    * Requires SD_SPI_BASE and SD_SPI_DATA_ACCESS.


See the implementation guide for details regarding the available functions and how to implement them.


## How to use
### NOTE: This is intended to run as is on an ATmega1280 Target.  It should be simple to run this from other AVR targets with a change in port assignments (SPI, USART), but it has only been tested against an ATmega1280 Target.  

To use, copy the source/header files and build/download the module using the AVR Toolchain.  
*  Primarily for Mac/Linux users, a "MAKE" file is included (for reference only) for building the module from the source files and downloading it to an AVR target - specifically ATmega1280.
* Windows users should be able to just build/download the module from the source files using Atmel Studio (though I have not used this).  

* A TEST.C is also provided, which includes main(), in order to demonstrate how to implement some of the functions.


## Additional Required Files
The following files are also required by the SD Card module.  These are included in this repository, though they are technically not considered part of the module itself.  These files are maintained in the [AVR-General](https://github.com/Jsfain/AVR-General.git)

1) SPI.C(H) - required to interface with the AVR's SPI port used for physical transmission and reception of data packets.
2) USART.C(H) - required to interface with the AVR's USART port used to print messages and data to a terminal.
3) PRINTS.H(C) - required to print integers (decimal, hex, binary) and strings to the screen via the USART.


## Warning
Use at your own risk. It is possible to write and erase data on an SD card using this module, and I have done so many times.  Suggest always backing up any data you do not want to lose before using.


## Disclaimers
* This is just a project I did for fun during quaratine. It has been tested against SDSC and SDHC type micro SD cards, and I expect it to also operate against full-sized cards as well, but no guarantee of functionality is made. I may or may not update in the future, though any feedback or suggestions are welcome.
* This module has only been tested against an ATmega1280 target, though it is expected it would be easily portable to other AVR targets with modification of SPI and USART ports, assuming sufficient memory.


# Who can use
Anyone. Use it. Modify it for specific purposes/systems, and if you want you can let me know if you found it helpful.


# Implementation
See the Implementation Guide for specifc instructions on implementation.


# Reference Documents:
1) Atmel ATmega640/V-1280/V-1281/V-2560/V-2561/V Datasheet
2) SD Specifications Part 1: Physical Layer Specification - Simplified Specification Version 7.10
