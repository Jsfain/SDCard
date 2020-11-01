# AVR-SD Card Module
Use an AVR microcontroller to execute SD card commands and handle the responses via SPI.


## Purpose
This was just something fun to work on during quarantine. It's purpose is to establish a set of functions for SD card data access and control in SPI mode using an AVR microcontroller. The module is originally developed to be a physical disk layer operating underneath a file system layer, but it may be simply implemented as a stand-alone SD card raw data access and control module.  


## Technology
* SD Specifications Part 1: Physical Layer Specification - Simplified Specification Version 7.10
* TARGET: ATmega1280 - expect this to be easily implemented on other AVR targets, with modification of port assignments and settings, assuming sufficient memory and resources are available.
* LANGUAGE: C
* [AVR-Toolchain](https://github.com/osx-cross/homebrew-avr) 9.3.0 , This includes: 
  * AVR-GCC 9.3.0: required to compile/build the module.
  * AVRDUDE 6.3: required to download the program to the AVR.



## Overview
The module is composed of three source/header files.  These are listed below in order of importance:

### AVR-SD Card Module
1. SD_SPI_BASE.C(H)
    * This includes the basic functions for direct interaction with the SD card and is required by all other module files. Among the included functions are those for initializing the SD card into SPI mode, sending commands to the SD card, and receiving responses in byte-sized packets from the SD card.

2. SD_SPI_DATA_ACCESS.C(H)
    * Includes some specific functions for handling data access such as reading, writing, and erasing SD card blocks.
    * Requires SD_SPI_BASE to handle sending commands to, and receiving responses from, the SD card via SPI.

3. SD_SPI_MISC.C(H)
    * Intended as a catch-all for miscellaneous functions that don't really fit in the other module files.
    * Requires SD_SPI_BASE and SD_SPI_DATA_ACCESS.


### Additional Required Files
The following source/header files are also required.  These are included in the repository but maintained in [AVR-General](https://github.com/Jsfain/AVR-General.git)

1. SPI.C(H)     : required to interface with the AVR's SPI port used for the physical sending/receiving of bytes to/from the SD card.
2. USART.C(H)   : required to interface with the AVR's USART port used to print messages and data to a terminal.
3. PRINTS.H(C)  : required to print integers (decimal, hex, binary) and strings to the screen via the USART.


## How to use
Copy the files and build/download the module using the AVR Toolchain. These are written for an ATmega1280 target, so if using a different target you will need to modify the code accordingly.  
 * The source files contain descriptions of each function available in the module.
 * SD_TEST.C is probably the best way to see how to implment this module. This file contains main(), and includes several examples of function implementation that can be referenced.
 * A "MAKE.SH" file can also be referenced to see how I built the module from the source files and downloaded it to an ATmega1280 AVR target. This would primarily be useful for non-Windows users without access to Atmel Studio.
 * Windows users should be able to just build/download the module from the source files using Atmel Studio (though I have not used this). Note, any paths (e.g. the includes) will need to be modified for compatibility.


## Who can use
Anyone. Use it. Modify it for your specific purpose/system. If you want, you can let me know if you found it helpful.


## License
[MIT](https://github.com/Jsfain/AVR-SDCard/blob/master/LICENSE)


## Warnings / Disclaimers
1.    Use at your own risk. It is possible to erase or overwrite data, lockout the SD card, and/or place the SD card into a bad logical or physical state. I take no responsibility for the loss of data or property through the use of this SD card module. This was developed for fun and so it is offered "AS IS" to anyone that wants to use it, look at it, or modify it for their purposes. There is no guarantee of operability under any circumstance. 
2.    Backup Data! See 1.
3.    This module has only been tested on an ATmega1280 microcontroller. It is expected to be easily portable to other AVR's through simple port (e.g. SPI, USART) reassignments, provided the resources exist, but also see 1. 
4.    This module has only been tested against version 2.x, 2GB and 4GB micro-SD cards of type SDSC (standard capacity) and SDHC (high capacity). It is unknown how it will behave running against other SD card types, versions, and capacities. Again, see 1.


## References
1. [AVR-General](https://github.com/Jsfain/AVR-General.git)
2. [AVR-Toolchain](https://github.com/osx-cross/homebrew-avr)
3. Atmel ATmega640/V-1280/V-1281/V-2560/V-2561/V Datasheet
4. SD Specifications Part 1: Physical Layer Specification - Simplified Specification Version 7.10   
