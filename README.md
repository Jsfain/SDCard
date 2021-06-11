# SD Card Module - SPI Mode
Module for accessing and controling an SD Card in SPI mode, intended for use with a microcontroller.

**Note**: the original intent is to use it running against an ATMega1280 AVR microcontroller's SPI port, though it should be easily extensible to operate against other microcontroller types, AVR and non-AVR, though this has not been tested. This will be discussed briefly later.  

## Purpose
This project was something I worked on during quarantine. It's purpose was to establish a set of functions for accessing and controlling an SD card operating in SPI mode using a microcontroller. The capabilities provided in the files included in this module are intended to provide functionality for standalone raw data access, but it can easily be implemented as a disk access layer of a File System driver. 

The image below shows the results from a raw data block read and print. This is done by using the sd_ReadSingleBlock function to read a specified block into an array, and then passing the block array to the sd_PrintSingleBlock function. This particular image is the raw data contents of the first block of the root directory in a FAT32 volume.

![alt text](https://github.com/Jsfain/AVR-SDCard/blob/master/images/printSingleBlock.png)

images/printSingleBlock.png
## Technology
* TARGET     : ATmega1280. Portable to other AVR targets with correct PORT reassignments, provided sufficient resources available.
* LANGUAGE   : C
* COMPILER   : AVR-GCC 9.3.0
* DOWNLOADER : AVRDUDE 6.3
* Compiler and downloader used were available with the [AVR-Toolchain from Homebrew](https://github.com/osx-cross/homebrew-avr).
* SD card SPI mode functionality was implemented according to the *SD Specifications Part 1: Physical Layer Specification - Simplified Specification Version 7.10*


## Overview
There are multiple source/header files included as part of this SD Card module. Only *SD_SPI_BASE.C(H)* is required, which will include SD_SPI_CAR.H. The source/header files are listed below, with brief descriptions, in order of precendence. See the files themselves for full definitions regarding each of the available functions, structs, macros, etc... 

### SD Card Module Files
1. **SD_SPI_BASE.C(H)** - *REQUIRED*
    * These source/header files are the only ones required to implement the module.
    * These files implement the basic functions required to interact with the SD card in SPI mode. In particular they implement the SD card's SPI mode initialization function, ***sd_InitModeSPI***, as well as implement the functions required by the initialization function, such as *sd_SendByteSPI*, *sd_ReceiveByteSPI*, *sd_SendCommand*, etc... 
    * Additionally, also provided in these files are some error printing functions for interpreting the output of R1 responses and the initialization routine.
    * SD_SPI_BASE.H will include SD_SPI_CAR.H which provides macro definitions for the SD card (C)ommands, (A)rguments, and (R)esponses available when operating in SPI mode. 
    * See the *SD_SPI_BASE* files for the full descriptions of the structs, functions, and macros available.

2. **SD_SPI_RWE.C(H)** - (R)ead/(W)rite/(E)rase
    * Requires SD_SPI_BASE.
    * These files provide command functions for the SD card to perform single-block reads and writes and multi-block erases.
    * Each of these command functions has a corresponding error print function included in the files.
    * These files also include a single-block print function ***sd_PrintSingleBlock*** that can be used to print a block of data read in by the single-block read function (see screenshot above). This function will print a specified block's contents in 16 byte rows of hex values and their corresponding ASCII characters, with the block-offset byte address beginning each row.
    * See the *SD_SPI_RWE* files for the full descriptions of the structs, functions, and macros available.

3. **SD_SPI_MISC.C(H)** - miscellaneous functions
    * Requires SD_SPI_BASE and SD_SPI_RWE.
    * These files are intended as a catch-all for miscellaneous functions.
    * The functions currently available in these files are mostly useful for demonstrating/testing how to execute certain SD card commands, and do not necessarily provide much practical purpose in their current implementation.
    * Currently these include multi-block read, write, and print functions, card capacity calculation functions, and others.
    * See the *SD_SPI_MISC* files for the full descriptions of the structs, functions, and macros available.

### Additional Required Files
The following source/header files are also required by the module, and thus included in the repository but they are maintained in [AVR-General](https://github.com/Jsfain/AVR-General.git)

1. SPI.C(H)     : required to interface with the AVR's SPI port used for the physical sending/receiving of data to/from the SD card.
2. USART0.C(H)   : required to interface with the AVR's USART port used to print messages and data to a terminal.
3. PRINTS.H(C)  : required to print integers (decimal, hex, binary) and strings to the screen via the USART.

#### additional comments:
The SD_SPI_XXXX files do not implement anything that directly interacts with the ATMega1280 AVR microcontroller, but instead use the SPI.C(H) and USART0.C(H) files for all interaction. As such, if a different microcontroller is to be used, then you would only need to ensure that the SPI and USART transmit and receive functions are implemented to behave similarly to those functions in the SPI and USART files included here. Additionally, the SD_SPI_XXXX files only utilize the USART through the PRINTS.H(C) and do not call any usart transmit \ receive functions directly. So, it should be fairly simple to implement this against a variety of microcontrollers, but that said it has only been tested with the ATMega1280.


## How to use
 * Clone the repo or copy the required files, then compile, build, and download to the AVR using your preferred method (Atmel Studio, AVR Toolchain, etc...). 
 * The source and header files contain descriptions of each function available and how to use them.

 ### SD_TEST.C
 * A test file, *SD_TEST.C*, is probably the best way to understand how to implment the functions in the module. This file contains main(), and includes several examples of implementing the various functions and capabilities available in the module.
 * *SD_TEST.C* is structured in sections to test out the various features and functions. These sections are independently enabled by setting local macros corresponding to a specific section to 1. The sections are clearly marked and the macros self-described.
 * The only section that is always enabled (no associated enabling macro) is the initialization section.
 * Below is an example of the initialization steps, similar to that implemented in *SD_TEST.C*. This, again, must always be implemented in main() prior to using any other parts of the AVR-SD Card module. 

### Initialization:
When writing a program to implement the AVR-SD Card module, the following must occur before anything else:
  1. Initialize USART0 with ***usart_Init()*** - *Required for any print functions.*
  2. Create an instance of a CTV struct (i.e. (C)ard (T)ype (V)ersion).
  3. Initialize the SD Card into SPI mode by calling ***sd_InitModeSPI(&ctv)*** - Pass a pointer of the CTV instance to the initialization routine, which will set its members to their correct values.
    * The SPI port will be initialized by the  ***spi_MasterInit()*** function which is called from this SD initialization function - *Required for master mode communincation on the SPI port.*
    * The CTV instance members should only be set one time and this should only be done by the initialization routine.
    * The *type* member of CTV will be used for determining whether the card should be block or byte addressed.
    * If the initialization function returns OUT_OF_IDLE, with no other errors, then initialization was successful.

**Example - Initialization**
 
```
int main(void)
{
  usart_Init();

  CTV ctv;

  uint32_t initResp = sd_InitModeSPI(&ctv);

  if (initResp != OUT_OF_IDLE)
  {   
      // Initialization failed
      sd_PrintInitError(initResp);
      sd_PrintR1(initResp);
  }
  else
  {   
    // initialization success!!
  }
}
```

 ### Additional Comments
 * A *MAKE.SH* file is included for reference to see how I built the module from the source files and downloaded it to an ATmega1280 AVR target. The make file would primarily be useful for non-Windows users without access to Atmel Studio. Windows users should be able to just build/download the module from the source/header files using Atmel Studio (though I have not used this).

## Who can use
Anyone.


## License
[GNU GPLv3](https://github.com/Jsfain/AVR-SDCard/blob/master/LICENSE)


## Warnings / Disclaimers
1. Use at your own risk. It is possible to erase or overwrite data, lockout an SD card, and/or place the SD card into a bad logical or physical state. I take no responsibility for the loss of data or property through the use of this SD card module. This was developed for fun and so it is offered "AS IS" to anyone that wants to use it. There is no guarantee of operability under any circumstance.
2. Backup Data! See 1.
3. This module has only been tested on an ATmega1280 microcontroller. It is expected to be easily portable to other targets, provided the resources exist, but also see 1.
4. This module has only been tested against version 2.x, 2GB and 4GB micro-SD cards of type SDSC (standard capacity) and SDHC (high capacity). It is unknown how it will behave running against other SD card types, versions, and capacities. Again, see 1.
5. Just be aware, this was built on a Mac, some of the interactive features in the test file may not behave the same as in other environments, in particular the interpretation of some characters that are sent and/or printed to the screen via the usart or possible directory/file paths, if they have been specified, such as those in the MAKE.SH file.


## References and Resources
1. [AVR-General](https://github.com/Jsfain/AVR-General.git)
2. [AVR-Toolchain](https://github.com/osx-cross/homebrew-avr)
3. Atmel ATmega640/V-1280/V-1281/V-2560/V-2561/V Datasheet
4. SD Specifications Part 1: Physical Layer Specification - Simplified Specification Version 7.10   
