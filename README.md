# AVR-SD Card Module
For executing SPI mode SD card commands from an AVR microcontroller host.


## Purpose
To establish a set of functions for SD card access and control in SPI mode from an AVR microcontroller. The module is originally developed to be a physical disk layer operating underneath a file system layer, but it may be simply implemented as a standalone SD card raw data access and control module.


## Technology Details
* TARGET: ATmega1280.  Should be easily implemented on other AVR targets with modification of port assignments and settings, and assuming memory is sufficient.
* LANGUAGE: C
* [AVR-Toolchain](https://github.com/osx-cross/homebrew-avr) 9.3.0: includes AVR-GCC and AVRDUDE. 
* AVR-GCC 9.3.0: required to compile and build the module. Included in the AVR-Toolchain.
* AVRDUDE 6.3: required to download the build to the AVR. Included in the AVR-Toolchain.


## Overview
This module was developed based on the SD Specifications Part 1: Physical Layer Specification - Simplified Specification Version 7.10

The module is composed of three source/header files.  These are listed below in order of importance/role:

### AVR-SDCard Module
1) SD_SPI_BASE.C(H)
    * Only this source/header is required.
    * Includes the basic functions for direct interaction with the SD card e.g. a function for initializing the SD card into SPI mode, for sending commands to the SD card, and for receiving responses in byte-sized packets from the SD card, among others.

2) SD_SPI_DATA_ACCESS.C(H)
    * Includes some specific functions for handling data access such as reading, writing, and erasing data blocks as well as some error print functions.. 
    * Requires SD_SPI_BASE to handle sending commands and receiving responses.

3) SD_SPI_MISC.C(H)
    * Intended to hold some miscellaneous functions. 
    * Requires SD_SPI_BASE and SD_SPI_DATA_ACCESS.


### Additional Required Files
The following source/header files are also required.  These are included in the repository, though they are technically not considered part of the module itself.  These files are maintained in [AVR-General](https://github.com/Jsfain/AVR-General.git)

1) SPI.C(H) - required to interface with the AVR's SPI port used for the physical sending/receiving bytes to/from the SD card.
2) USART.C(H) - required to interface with the AVR's USART port used to print messages and data to a terminal.
3) PRINTS.H(C) - required to print integers (decimal, hex, binary) and strings to the screen via the USART.


## How to use
To use, copy the source/header files and build/download the module using the AVR Toolchain (specifically intended for ATmega1280).  
 * See the AVR-SDCard Module Guide for specifics on how to use the functions and other details.
 * SD_TEST.C file contains main(), and also includes several examples of function implementation that can be referenced.
 * A "MAKE" file is included (for reference only) for building the module from the source files and downloading it to an ATmega1280 AVR target (primarily for non-windows users).
 * Windows users should be able to just build/download the module from the source files using Atmel Studio (though I have not used this). Note, any paths (e.g. the includes) will need to be modified for compatibility.


## Who can use
Anyone. Use it. Modify it for specific purposes/systems, and if you want you can let me know if you found it helpful.


## License


## Warnings / Disclaimers
1.    Use at your own risk. This SD card module was developed for fun and so it is offered as is to anyone that wants to use it, look at it, or modify it for their needs. There is no guarantee of operability under any circumstance and it is possible to erase or overwrite data, lockout the SD card, and/or get the SD card into a bad logical or physical state. I take no responsibility for the loss of data or property through the use of this SD card module.
2.    Backup Data! See 1.
3.    This module has only been tested on an ATmega1280 microcontroller (µC). It is expected to be easily portable to other AVR µCs through simple port (e.g. SPI, USART) reassignments, provided the resources exist, but also see 1. 
4.    This module has only been tested against version 2.x, 2GB and 4GB micro-SD cards of type SDSC (standard capacity) and SDHC (high capacity). It is unknown how it will behave running against other SD card types, versions, and capacities. Again, see 1.


## References
1.  AVR-SDCard Module Guide - instruction guide for this module
2. [AVR-General](https://github.com/Jsfain/AVR-General.git) - respository where other non-module files are maintained
3. [AVR-Toolchain](https://github.com/osx-cross/homebrew-avr)  - required to build/download the files.  On Windows, Atmel Studio should come with the toolchain.
3. Atmel ATmega640/V-1280/V-1281/V-2560/V-2561/V Datasheet
4. SD Specifications Part 1: Physical Layer Specification - Simplified Specification Version 7.10
