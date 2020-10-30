# AVR-SD Card Module
Use and AVR microcontroller to execute SPI mode SD card commands.


## Purpose
To establish a set of functions for SD card access and control in SPI mode using an AVR microcontroller. The module is originally developed to be a physical disk layer operating underneath a file system layer, but it may be simply implemented as a stand-alone SD card raw data access and control module.  This was just something fun to work on during quarantine.


## Technology
* TARGET: ATmega1280 - This is expected to be easily implemented on other AVR targets with modification of port assignments and settings, and assuming memory and other resources are sufficient.
* LANGUAGE: C
* [AVR-Toolchain](https://github.com/osx-cross/homebrew-avr) 9.3.0 , This includes: 
  * AVR-GCC 9.3.0: required to compile/build the module.
  * AVRDUDE 6.3: required to download the to the AVR.


## Overview
This module was developed by referencing the SD Specifications Part 1: Physical Layer Specification - Simplified Specification Version 7.10

The module is composed of three source/header files.  These are listed below in order of importance/role:

### AVR-SD Card Module
1. SD_SPI_BASE.C(H)
    * This source/header is required byt all other module files.
    * These files include the basic functions for direct interaction with the SD card, among these are functions for initializing the SD card into SPI mode, for sending commands to the SD card, and for receiving responses in byte-sized packets from the SD card.

2. SD_SPI_DATA_ACCESS.C(H)
    * Includes some specific functions for handling data access such as reading, writing, and erasing data blocks. It also includes some error print functions as well.
    * Requires SD_SPI_BASE to handle sending commands and receiving responses.

3. SD_SPI_MISC.C(H)
    * Intended to hold some miscellaneous functions, that don't really fit anywhere else. 
    * Requires SD_SPI_BASE and SD_SPI_DATA_ACCESS.


### Additional Required Files
The following source/header files are also required.  These are included in the repository and maintained in [AVR-General](https://github.com/Jsfain/AVR-General.git)

1. SPI.C(H)     : required to interface with the AVR's SPI port used for the physical sending/receiving of bytes to/from the SD card.
2. USART.C(H)   : required to interface with the AVR's USART port used to print messages and data to a terminal/screen.
3. PRINTS.H(C)  : required to print integers (decimal, hex, binary) and strings to the screen via the USART.


## How to use
Copy the files and build/download the module using the AVR Toolchain. These are written for an ATmega1280 target, so if using a different target then you will need to modify the code accordingly.  
 * The source files contain descriptions of each function available in the module.
 * SD_TEST.C is probably the best way to see how to implment this module. This file contains main(), and includes several examples of function implementation that can be referenced.
 * A "MAKE.SH" file can also be referenced for seeing how I built the module from the source files and downloaded it to an ATmega1280 AVR target. This would primarily be useful for non-Windows users without access to Atmel Studio (like me).
 * Windows users should be able to just build/download the module from the source files using Atmel Studio (though I have not used this). Note, any paths (e.g. the includes) will need to be modified for compatibility.


## Who can use
Anyone. Use it. Modify it for your specific purpose/system. If you want, you can let me know if you found it helpful.


## License
[MIT](https://github.com/Jsfain/AVR-SDCard/blob/master/LICENSE)


## Warnings / Disclaimers
1.    Use at your own risk. It is possible to erase or overwrite data, lockout the SD card, and/or get the SD card into a bad logical or physical state. I take no responsibility for the loss of data or property through the use of this SD card module. This was developed for fun and so it is offered "as is" to anyone that wants to use it, look at it, or modify it for their purposes. There is no guarantee of operability under any circumstance. 
2.    Backup Data! See 1.
3.    This module has only been tested on an ATmega1280 microcontroller. It is expected to be easily portable to other AVR's through simple port (e.g. SPI, USART) reassignments, provided the resources exist, but also see 1. 
4.    This module has only been tested against version 2.x, 2GB and 4GB micro-SD cards of type SDSC (standard capacity) and SDHC (high capacity). It is unknown how it will behave running against other SD card types, versions, and capacities. Again, see 1.


## References
1. [AVR-General](https://github.com/Jsfain/AVR-General.git)
2. [AVR-Toolchain](https://github.com/osx-cross/homebrew-avr)
3. Atmel ATmega640/V-1280/V-1281/V-2560/V-2561/V Datasheet
4. SD Specifications Part 1: Physical Layer Specification - Simplified Specification Version 7.10   
