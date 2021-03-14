/*
 * File    : SD_SPI_REGS.H
 * Version : 0.0.0.1 
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : MIT
 * Copyright (c) 2020-2021
 * 
 * Macro definitions for SD Card registers.
 * 
 * This file should only be included using sd_spi_base.h
 */

#ifndef SD_SPI_REGS_H
#define SD_SPI_REGS_H

/*
 ******************************************************************************
 *                      OPERATION CONDITIONS REGISTER (OCR)
 ******************************************************************************
 */
#define POWER_UP_BIT_MASK     0x80
#define CCS_BIT_MASK          0x40          // Card Capacity Support
#define UHSII_BIT_MASK        0x20          // UHS-II Card Status
#define CO2T_BIT_MASK         0x10          // Over 2TB support status
#define S18A_BIT_MASK         0x08          // switching to 1.8V accepted

// Volt Range Accepted by card: 2.7 - 3.6V. Only this range currently supported
#define VRA_OCR_MASK          0xFF80

#endif//SD_SPI_REGS_H
