# SD Card - SPI Mode
Module for accessing and controlling an SD Card in SPI mode. 

***Quick Note on Portability***

*This SD Card module is intended to work with the SPI port on an **ATMega1280 AVR microcontroller**. However, any of the AVR-specific functionality, is handled entirely within the AVR IO port access files found under **avrio** within this repo. These IO files, **AVR_SPI** and **AVR_USART** are maintained in [AVR-IO](https://github.com/Jsfain/AVR-IO). As such, it should be straightforward to implement the SD Card module to operate against other target devices, ensuring the few SPI- and USART-specific macros and functions required are included. **AVR_SPI** is included by SD_SPI_BASE and **AVR_USART** is included by PRINTS helper and is only necessary if using any of the printing functions. See the SD_SPI_BASE and the PRINTS files for specific details.*

## Purpose
This module was developed to establish a set of functions for accessing and controlling an SD Card in SPI mode using an SPI-capable microcontroller. The capabilities provided in this module allow for executing any of the SPI-specific SD card commands. As such, this module could be used as a stand-alone SD card driver for raw data access or implemented as a disk access layer under a file system driver.

As a simple example of its use, the image below shows the results of a raw data block read and print. This is done using the ***sd_ReadSingleBlock*** function to read the specified data block into an array, then calling the ***sd_PrintSingleBlock*** function. The image below is of the raw data contents of the first block of the root directory in a FAT32 volume, showing the Block Address Offset, and the raw data as both HEX and ASCII.

![alt text](https://github.com/Jsfain/SDCard/blob/master/images/printSingleBlock.png)


## Who can use
Anyone.

## How to use
 * The source and header files contain descriptions of each function available and how to use them.
 * If using against an AVR ATMega1280 target, simply clone/copy the repo, compile/build, and then download to the microcontroller. 
 * If **NOT** using an AVR ATMega1280 then it will be necessary to either modify, or replace, the IO-specific files that have been included in the repo (AVR_SPI and AVR_USART), to support the desired target. See ***Portability Considerations*** section below.

## Technology
* LANGUAGE      : C
* TARGET(S)     : ATmega1280 - only target tested. 
* COMPILER(S)   : AVR-GCC 9.3.0
* DOWNLOADER(S) : AVRDUDE 6.3
* Compiler and downloader used were available with the [AVR-Toolchain from Homebrew](https://github.com/osx-cross/homebrew-avr).
* SD card SPI mode functionality was implemented according to the *SD Specifications Part 1: Physical Layer Specification - Simplified Specification Version 7.10*


## Overview of Contents
There are multiple source/header files included as part of this SD Card module. Only *SD_SPI_BASE.C(H)* is required, which will include SD_SPI_CAR.H. The source/header files are listed below, with brief descriptions, in order of precendence. See the files themselves for full definitions regarding each of the available functions, structs, macros, etc... 

### SD Card Module Files
1. **SD_SPI_BASE.C(H)** - *REQUIRED*
    * These source/header files are the only ones required to implement the module; provided SPI and USART functionality is properly handled.
    * These files implement the basic functions required to interact with the SD card in SPI mode. In particular they implement the SD card's SPI mode initialization function, ***sd_InitModeSPI***, as well as implement the functions required by the initialization function, such as *sd_SendByteSPI*, *sd_ReceiveByteSPI*, *sd_SendCommand*, etc... 
    * SD_SPI_BASE.H will include SD_SPI_CAR.H which provides macro definitions for the SD card (C)ommands, (A)rguments, and (R)esponses available for SD cards operating in SPI mode.
    * See the *SD_SPI_BASE* files for more detailed descriptions of the specific structs, functions, and macros available, as well as what functions and macros must be implemented by the SPI interface for portability considerations.

2. **SD_SPI_RWE.C(H)** - (R)ead/(W)rite/(E)rase
    * Requires SD_SPI_BASE.
    * These files provide command functions for the SD card to perform single-block reads and writes and multi-block erases.
    * See the *SD_SPI_RWE* files for the full descriptions of the structs, functions, and macros available.

3. **SD_SPI_PRINT.C(H)** - SD print functions
    * These source/header files can be used to print out the various error reponses from ***SD_SPI_BASE*** and ***SD_SPI_RWE***.
    * They also include ***sd_PrintSingleBlock*** used to print a block of data read in by the single-block read function from ***SD_SPI_RWE*** (see screenshot above). The function prints the provided block data in 16 byte rows of hex values and their corresponding ASCII characters, with the block-offset byte address beginning each row.
    * See the *SD_SPI_PRINT* files for the full descriptions of the structs, functions, and macros available.

4. **SD_SPI_MISC.C(H)** - miscellaneous functions
    * Requires SD_SPI_BASE, SD_SPI_RWE, and SD_SPI_PRINT
    * These files are intended as a catch-all for miscellaneous functions.
    * The functions currently available in these files are mostly useful for demonstrating/testing how to execute certain SD card commands, and do not necessarily provide much practical purpose in their current implementation.
    * Currently these include multi-block read, write, and print functions, card capacity calculation functions, and some others.
    * See the *SD_SPI_MISC* files for the full descriptions of the structs, functions, and macros available.

### Helper Files
1. **PRINTS.H(C)** : This file is only needed if any of the SD print functions/files are to be used. This is a simple file used to print integers (decimal, hex, binary) and strings to the screen via a U(S)ART. Any source files that include print functions require this to be implemented. In it's current implementation it includes AVR_USART.C(H) for transmiting bytes to print via USART. See the file itself for portability considerations. The file is maintained in [C-Helpers](https://github.com/Jsfain/C-Helpers)

### IO Files
The following source/header files are also used by the module for SPI and USART access for the specific AVR target, and so have been included in the repository but they are maintained in [AVR-IO](https://github.com/Jsfain/AVR-IO.git)

1. AVR_SPI.C(H)    : Used to interface with the AVR's (ATMega1280) SPI port for the physical sending/receiving of data to/from the SD card.
2. AVR_USART.C(H)  : Used to interface with the AVR's (ATMega1280) USART port used to print messages and data to a terminal. This is only needed if the provided SD print functions/files are to be used in SD_SPI_PRINT and SD_SPI_MISC.


 ### SD_TEST.C
 * A test file, *SD_TEST.C*, is also included and is probably the best way to understand how many of the functions in the module are implemented. The file contains main(), and includes several examples of implementing the various functions and capabilities available in the module.
 * *SD_TEST.C* is structured in sections to test out the various features and functions. These sections are independently enabled by setting local macros to 1, that correspond to a specific section to 1. The sections are clearly marked and the macros self-described (see the file).
 * The only section that is always enabled (no associated enabling macro) is the initialization section.
 * Below is an example of the initialization steps, similar to that implemented in *SD_TEST.C*. This, again, must always be implemented in main() prior to using any other parts of this SD Card module. 

### SD Card Initialization example:
The below steps outline the process for initializing the SD card before it can be utilized. Steps 2 and 3 below are required. Step 1 (USART initialization) is only necessary if using any print functions, which this example uses:
  1. Initialize USART with ***usart_Init()*** - *Required for any print functions.*
  2. Create instance of CTV struct (i.e. (C)ard (T)ype (V)ersion).
  3. Initialize the SD Card into SPI mode by calling ***sd_InitModeSPI(&ctv)*** - Pass a pointer of the CTV instance to the initialization routine, which will set its members to their correct values.
    * The SPI port will be initialized from this function into master mode. See the file for details if interfacing with a target other than ATMega1280.
    * The CTV instance members should only be set once, and this should only be done by the initialization routine.
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
 * A *MAKE.SH* file is included for reference only. This is simply to see how I built the module from the source files and downloaded it to an ATmega1280 AVR target. The make file would primarily be useful for non-Windows users without access to Atmel Studio. Windows users should be able to just build/download the module from the source/header files using Atmel Studio (though I have not used this).


## License
[GNU GPLv3](https://github.com/Jsfain/SDCard/blob/master/LICENSE)


## Warnings / Disclaimers
1. Use at your own risk. It is possible to erase or overwrite data, lockout an SD card, and/or place the SD card into a bad logical or physical state. I take no responsibility for the loss of data or property through the use of this SD card module. This was developed for fun and so it is offered "AS IS" to anyone that wants to use it. There is no guarantee of operability under any circumstance.
2. Backup Data! See 1.
3. This module has only been tested on an ATmega1280 microcontroller. It is expected to be easily portable to other targets, provided the resources exist, but also see 1.
4. This module has only been tested against version 2.x, 2GB and 4GB micro-SD cards of type SDSC (standard capacity) and SDHC (high capacity). It is unknown how it will behave running against other SD card types, versions, and capacities. Again, see 1.
5. Just be aware, this was built on a Mac, some of the interactive features in the test file may not behave the same as in other environments, in particular the interpretation of some characters that are sent and/or printed to the screen via the usart or possible directory/file paths, if they have been specified, such as those in the MAKE.SH file.


## References and Resources
1. [AVR-IO](https://github.com/Jsfain/AVR-General.git)
2. [C-Helpers](https://github.com/Jsfain/C-Helpers)
3. [AVR-Toolchain](https://github.com/osx-cross/homebrew-avr)
4. Atmel ATmega640/V-1280/V-1281/V-2560/V-2561/V Datasheet
5. SD Specifications Part 1: Physical Layer Specification - Simplified Specification Version 7.10   
