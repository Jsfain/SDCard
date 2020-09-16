# AVR
Repository for modules developed to run on AVR microcontrollers.

# Purpose
To create a centralized repository as a hub for useful AVR modules that can then be utilized in other programs.

# Repository Structure Details
* This repository contains two subdirectories - GENERAL and MODULES. 
    * MODULES contains categorized subdireoctories which contain specific modules,  e.g. DISK_IO subdirectory category contains the SDCard module. 
    * GENERAL contains source and headers files for implementing common standard features of an AVR microcontroller (e.g. USART, SPI) and simple helper programs that are used throughout the MODULES - e.g. PRINTS which contains a few print functions used by all of the modules.
* See README in each module for details on a specific module.

# Additional Notes

# Who can use
Anyone can use these and modify them to meet the needs of their system, but I would appreciate acknowledgement and/or that you let me know if you find it helpful.

# Requirements
[AVR Toolchain](https://github.com/osx-cross/homebrew-avr)