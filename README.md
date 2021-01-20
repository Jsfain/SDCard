# AVR-SD Card
Access and control an SD Card using an AVR microcontroller in SPI mode.


## Purpose
This projects was something I decided to work on during quarantine. It's purpose was to establish a set of functions for accessing and controlling an SD card operating in SPI mode using an AVR microcontroller's SPI port. The capabilities provided in the files included in this module can be used to develop an SD card raw data access driver and/or used to develop the physical disk driver for a file system driver/layer.


## Technology
* Designed following the SPI port specifications provided in the *SD Specifications Part 1: Physical Layer Specification - Simplified Specification Version 7.10*
* TARGET: *ATmega1280* - should be easily extended to other AVR targets with sufficient memory/resources. May require modification of port assignments and settings.
* LANGUAGE: *C*
* Built on a macOS using the [AVR-Toolchain](https://github.com/osx-cross/homebrew-avr) 9.3.0, includes: 
  * compiler: *AVR-GCC - 9.3.0*
  * downloader: *AVRDUDE - 6.3*


## Overview
There are multiple source/header files that are part of this AVR-SD Card module. Only *SD_SPI_BASE.C(H)* is required, which includes SD_SPI_CMDS.H. The source/header files are listed below, with brief descriptions, in order of precendence. See the files themselves for full definitions regarding each of the available functions, structs, and macros. 

### AVR-SD Card Module Files
1. **SD_SPI_BASE.C(H)** - *REQUIRED*
    * The only files required to implement the module.
    * This will include the SD_SPI_CMDS.H file which provides the macro definitions for the SD card commands available when operating in SPI mode. 
    * These files implement the basic functions required to interact with the SD card in SPI mode. In particular they implement the SD card's SPI mode initialization function, ***sd_spiModeInit(CTV * ctv)***, as well as implement functions required by the initialization function, such as *sd_sendByteSPI()*, *sd_receiveByteSPI()*. 
    * Additionally, there is also provided some error printing functions for interpreting the output of R1 responses and the initialization routine.
    * See the *SD_SPI_BASE* files for a complete list of functions, structs, and macros and their full description.

2. **SD_SPI_RWE.C(H)** - read/write/erase
    * These files define functions for SD card to send single-block read, write and multi-block erase commands and arguments, and handle the responses to each of these commands by the SD card.
    * Each of these command functions has a corresponding error print function included in the files.
    * These files also include a single-block print function that can be used to print a block of data read in by the single-block read function. This function will print the block's contents in 16 byte rows of hex values and their corresponding ASCII characters, with the block offset address beginning each row.
    * These files require SD_SPI_BASE to handle the card initialization as well as the actual sending of the single-block read, write, and multi-block erase commands and arguments to the SD card in the correct format.
    * See the *SD_SPI_RWE* files for a complete list of functions, structs, and macros and their full description provided in these files.

3. **SD_SPI_MISC.C(H)** - miscellaneous
    * These files are intended as a catch-all for miscellaneous functions, and I'm less diligent about maintaining these at the moment.
    * The functions currently available here are mostly useful for demonstrating/testing how to issue certain SD card commands, and do not necessarily provide much practical purpose in their current implementation.
    * Currently these include multi-block read, write, and print functions, as well as card capacity calculation functions.
    * See the *SD_SPI_MISC* files for a complete list of functions, structs, and macros included here.

### Additional Required Files

The following source/header files are also required.  These are included in the repository but maintained in [AVR-General](https://github.com/Jsfain/AVR-General.git)

1. SPI.C(H)     : required to interface with the AVR's SPI port used for the physical sending/receiving of bytes to/from the SD card.
2. USART.C(H)   : required to interface with the AVR's USART port used to print messages and data to a terminal.
3. PRINTS.H(C)  : required to print integers (decimal, hex, binary) and strings to the screen via the USART.


## How to use
 * Clone the repo and/or copy the required files, then build and download it using your preferred method (Atmel Studio, AVR Toolchain, etc...). 
 * The source and header files contain full descriptions of each function and what they do.

 ### SD_TEST.C
 * A test file, *SD_TEST.C*, is probably the best way to understand how to implment the module. This file contains main(), and includes several examples of implementing the various functions available.
 * *SD_TEST.C* is structured in sections to test out the various features and functions. These sections are clearly marked and described in block comments at the top of each section. Each section can be commented out independently of the other sections so you can easily test the various functions. This is, of course, with the exception of the initialization section which must always be implmented.
 * Below is an example of the initialization steps, similar to that implemented in *SD_TEST.C*. This, again, must always be implemented in main() prior to using any other parts of the AVR-SD Card functions. 

### Initialization:
When writing a program to implement the AVR-SD Card functions, the following must occur before anything else:
  1. Initialize USART0 with ***usart_init()*** - *Required for any print functions.*
  2. Initialize SPI port with ***spi_masterInit()*** - *Required for communincation on the AVR's SPI port.*
  3. Create an instance of a CTV struct (i.e. (C)ard (T)ype (V)ersion).
  4. Initialize the SD Card into SPI mode by calling ***sd_spiModeInit(*ctv)*** - Pass a pointer of the CTV instance to the initialization routine, which will set its members to their correct values.
    * The CTV instance members should only be set one time and this should only be done by the initialization routine.
    * The *type* member of CTV will be used for determining whether the card should be block or byte addressed.

**Example**
 
```
int main(void)
{
  uint32_t initResp;                  
  CTV ctv;                 

  usart_init();
  spi_masterInit();

  initResp = sd_spiModeInit(&ctv);

  if (initResp != 0)
  {   
      // Initialization failed
      sd_printInitError(initResp);
      sd_printR1(initResp);
  }
  else
  {   
    // initialization success!!
  }
}
```


 ### Additional Comments
 * A "MAKE.SH" file can also be referenced to see how I built the module from the source files and downloaded it to an ATmega1280 AVR target. The make file would primarily be useful for non-Windows users without access to Atmel Studio. Windows users should be able to just build/download the module from the source files using Atmel Studio (though I have not used this).
 * All of these files are intended to be implemented on an ATmega1280 target, implementing this against a different AVR target will likely require modifying the code. This should only require modification to the PORT definitions, but, again, this has not been tested. 


## Who can use
Anyone. Use it. Modify it for your specific purpose/system. If you want, you can let me know if you found it helpful.


## License
[MIT](https://github.com/Jsfain/AVR-SDCard/blob/master/LICENSE)


## Warnings / Disclaimers
1. Use at your own risk. It is possible to erase or overwrite data, lockout an SD card, and/or place the SD card into a bad logical or physical state. I take no responsibility for the loss of data or property through the use of this AVR-SD card module/files. This was developed for fun and so it is offered "AS IS" to anyone that wants to use it, look at it, or modify it for their purposes. There is no guarantee of operability under any circumstance.
2. Backup Data! See 1.
3. This module has only been tested on an ATmega1280 microcontroller. It is expected to be easily portable to other AVR's through simple port (e.g. SPI, USART) reassignments, provided the resources exist, but also see 1.
4. This module has only been tested against version 2.x, 2GB and 4GB micro-SD cards of type SDSC (standard capacity) and SDHC (high capacity). It is unknown how it will behave running against other SD card types, versions, and capacities. Again, see 1.
5. Just be aware, this was built on a Mac, some of the interactive features in the test file may not behave the same as in other environments, in particular the interpretation of some characters that are sent and/or printed to the screen via the usart or possible directory/file paths, if they have been specified, such as in the MAKE.SH file.


## References
1. [AVR-General](https://github.com/Jsfain/AVR-General.git)
2. [AVR-Toolchain](https://github.com/osx-cross/homebrew-avr)
3. Atmel ATmega640/V-1280/V-1281/V-2560/V-2561/V Datasheet
4. SD Specifications Part 1: Physical Layer Specification - Simplified Specification Version 7.10   
