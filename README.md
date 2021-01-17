# AVR-SD Card Module
Use an AVR microcontroller to execute SD card commands in SPI mode.


## Purpose
This was just something fun to work on during quarantine. It's purpose was to establish a set of functions for SD card data access and control in SPI mode using an AVR microcontroller. This can be implemented on its own for raw data access of an SD card, or as the physical disk layer in a file system module.


## Technology
* SD Specifications Part 1: Physical Layer Specification - Simplified Specification Version 7.10
* TARGET: ATmega1280 - should be easily implemented on other AVR targets with sufficient memory and resources, but may require modification of port assignments and settings.
* LANGUAGE: C
* Built on a Mac using the [AVR-Toolchain](https://github.com/osx-cross/homebrew-avr) 9.3.0, This includes: 
  * AVR-GCC 9.3.0
  * AVRDUDE 6.3


## Overview
This full AVR-SD Card implementation is composed of multiple source and header files.  These are listed below in order of importance. Only SD_SPI_BASE.C(H) is required, which includes SD_SPI_CMDS.H.

### AVR-SD Card Module files
1. SD_SPI_BASE.C(H) - *REQUIRED*
    * Defines the SD card SPI mode initialization function, ***sd_spiModeInit(CTV * ctv)***, as well as the basic send command, send byte, and recieve byte functions.
    * This will include SD_SPI_CMDS.H

2. SD_SPI_RWE.C(H) - read/write/erase
    * Defines functions for single-block read, print, and write and multi-block erase.
    * This requires SD_SPI_BASE to handle sending commands to, and receiving responses from, the SD card via SPI.

3. SD_SPI_MISC.C(H)
    * Intended as a catch-all for miscellaneous functions.
    * Requires SD_SPI_BASE and SD_SPI_RWE.


### Additional Required Files
The following source and header files are also required.  These are included in the repository but maintained in [AVR-General](https://github.com/Jsfain/AVR-General.git)

1. SPI.C(H)     : required to interface with the AVR's SPI port used for the physical sending/receiving of bytes to/from the SD card.
2. USART.C(H)   : required to interface with the AVR's USART port used to print messages and data to a terminal.
3. PRINTS.H(C)  : required to print integers (decimal, hex, binary) and strings to the screen via the USART.


## How to use
 * Clone the repo and/or copy the required files, then build/download the module using your preferred method (Atmel Studio, AVR Toolchain, etc...). 
 * The source and header files contain full descriptions of each function and what they do.
 * A test file, **SD_TEST.C**, is probably the best way to understand how to implment the module. This file contains main(), and includes several function examples.
 * **SD_TEST.C** is structured in blocks to test out the various features and functions. These blocks are clearly marked and described in comments. The blocks can be commented out independently of each other so you can easily test the various functions. 
 * Below is an example of the simple initialization steps that must be implemented prior to using any other parts of the AVR-SD Card module files/functions. 

### Initialization:

When writing a program to implement the AVR-SD Card module, the following must occur before anything else:
  1. Initialize USART0 with **usart_init()** - Required for any print functions.
  2. Initialize SPI port with spi_masterInit() - Required for communincation on the AVR's SPI port.
  3. Create instance of CardTypeVersion struct (typedef CTV).
  4. Initialize the SD Card into SPI mode by calling **sd_spiModeInit(*ctv)** - Pass a pointer of the ctv instance to the initialization routine, which will set its members to their correct values.
    * The CTV instance members should only ever be set by the initialization routine.
    * The *ctv.type* member will be used for determining whether the card should be block or byte addressed.

**Example**
 
```
int main(void)
{
  uint32_t initResp;
  CTV ctv;

  usart_init();
  spi_masterInit();

  initResp = sd_spiModeInit (&ctv);

  if (initResp != 0)
  {   
      // Initialization failed
      sd_printInitError (initResp);
      sd_printR1 (initResp);
  }
  else
  {   
    // initialization success!!
  }
}
```

 ### Additional Comments
 * A "MAKE.SH" file can also be referenced to see how I built the module from the source files and downloaded it to an ATmega1280 AVR target. The make file would primarily be useful for non-Windows users without access to Atmel Studio. Windows users should be able to just build/download the module from the source files using Atmel Studio (though I have not used this).
 * All of these files are intended to be implemented on an ATmega1280 target, implementing this against a different AVR target will likely require modifying the code accordingly - This should only require modification to the PORT definitions, but this has not been tested. 


## Who can use
Anyone. Use it. Modify it for your specific purpose/system. If you want, you can let me know if you found it helpful.


## License
[MIT](https://github.com/Jsfain/AVR-SDCard/blob/master/LICENSE)


## Warnings / Disclaimers
1. Use at your own risk. It is possible to erase or overwrite data, lockout the SD card, and/or place the SD card into a bad logical or physical state. I take no responsibility for the loss of data or property through the use of this AVR-SD card module/files. This was developed for fun and so it is offered "AS-IS" to anyone that wants to use it, look at it, or modify it for their purposes. There is no guarantee of operability under any circumstance.
2. Backup Data! See 1.
3. This module has only been tested on an ATmega1280 microcontroller. It is expected to be easily portable to other AVR's through simple port (e.g. SPI, USART) reassignments, provided the resources exist, but also see 1.
4. This module has only been tested against version 2.x, 2GB and 4GB micro-SD cards of type SDSC (standard capacity) and SDHC (high capacity). It is unknown how it will behave running against other SD card types, versions, and capacities. Again, see 1.
5. Just be aware, this was built on a Mac, some of the interactive features in the test file may not behave the same as in other environments, in particular the interpretation of some characters that are sent and/or printed to the screen via the usart.


## References
1. [AVR-General](https://github.com/Jsfain/AVR-General.git)
2. [AVR-Toolchain](https://github.com/osx-cross/homebrew-avr)
3. Atmel ATmega640/V-1280/V-1281/V-2560/V-2561/V Datasheet
4. SD Specifications Part 1: Physical Layer Specification - Simplified Specification Version 7.10   
