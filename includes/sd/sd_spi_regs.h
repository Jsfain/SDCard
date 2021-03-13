/*
 * File    : SD_SPI_REGS.H
 * Version : 0.0.0.1 
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : MIT
 * Copyright (c) 2020-2021
 * 
 * macro definitions for SD Card registers.
 */

#ifndef SD_SPI_REGS_H
#define SD_SPI_REGS_H

//
// OCR - OPERATION CONDITIONS REGISTER
//
#define POWER_UP_BIT_MASK     0x80
#define CCS_BIT_MASK          0x40          // Card Capacity Support
#define UHSII_BIT_MASK        0x20          // UHS-II Card Status
#define CO2T_BIT_MASK         0x10          // Over 2TB support status
#define S18A_BIT_MASK         0x08          // switching to 1.8V accepted

//
// Voltage Range Accepted by card. 0xFF80 is volt range of 2.7 to 3.6V accepted
// This is the full range and currently host will only support this setting.
//
#define VRA_OCR_MASK          0xFF80


#endif//SD_SPI_REGS_H
