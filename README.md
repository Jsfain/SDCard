# AVR
Repository for modules developed to run on AVR microcontrollers.
Target - ATmega1280 

# Purpose
Create a hub for interdependent modules developed for AVR microcontrollers which can then be impelmented in lareger AVR-specific programs.

# Details
Written in C
compiled using avr-gcc (avr-gcc -v : gcc version 5.4.0 - AVR_8_bit_GNU_Toolchain_3.6.2_503) 

# Additional Notes
* This hub contains two subdirectories - General and Modules. 
    * GENERAL contains source and headers for implementing common features of an AVR microcontroller, e.g. USART, SPI, 
    * MODULES contains category subdireoctories which contain specific modules,  e.g. DISK_IO category contains the SDCard module.  

# Caution
* The code/modules here have currently only been tested on an ATmega1280 microcontroller, however, as long as memory is succifient and the correct PORTS are used for the different functionality (e.g. USART, SPI, ... ), the modules used here should be easily portable to other AVR microcontrollers.

# Who can use
Anyone can use this and modify it to meet the needs of their system, but would appreciate acknowledgement and/or that you would let me know if you use it or find it helpful.

# Requirements
[AVR Toolchain](https://github.com/osx-cross/homebrew-avr)