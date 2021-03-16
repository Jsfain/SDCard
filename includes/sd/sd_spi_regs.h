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

/*
 ******************************************************************************
 *                      CARD-SPECIFIC DATA REGISTER (CSD)
 ******************************************************************************
 */

#define CSD_STRUCT_V_SDSC    0
#define CSD_STRUCT_V_SDHC    1
#define CSD_STRUCT_MASK      0x40

#define CSD_STRUCT_VSN_CALC(X)     ((X) & CSD_STRUCT_MASK ?                   \
                                    CSD_STRUCT_V_SDHC : CSD_STRUCT_V_SDSC)

//
// Bit 7 of TAAC is reserved so must be 0, all other bits could be 1 or 0.
// This TAAC check returns true (1) if the reserved bit is 0.
//
#define TAAC_SDSC_RSVD_BIT   0x80
#define TAAC_CHECK_SDSC(X)   (!((X) & TAAC_SDSC_RSVD_BIT))

#define TAAC_SDHC            0x0E // for SDHC, TAAC is fixed at 1ms (0x0E).

//#define NSAC_SDSC         TBD   // could be any value.
#define NSAC_SDHC            0    // Not used, but field is still there.

// This is default. Other values possible if set to higher speed mode.
#define TRANS_SPEED         0x32  

// CCC has form _1_1101101_1 for SDHC
#define CCC_HI_BYTE_SDHC_CHK(X)  ((X) | 0xA0) == 0xFB

// CCC has form 01_1101101_1 for SDSC
#define CCC_HI_BYTE_SDSC_CHK(X)  ((X) | 0x40) == 0x7B

//
// The lower 4 bits of the CCC are the upper 4 bits of this byte and must be
// 0x50. The lower 4 bits are the READ_BL_LEN value.
//
#define CCC_LO_NIB              0x50        // same for SDSC and SDHC

#define READ_BL_LEN_SDHC        0x09        // must be 9 for SDHC

// read block length field could be either 9, 10, 11. This gives the range.
#define RBL_SDSC_LO     0x09
#define RBL_SDSC_HI     0x0B

#define READ_BL_LEN_SDSC_MASK   0x0F

//
// READ_BLK_PARTIAL[7] = 0, WRITE_BLK_MISALIGN[6] = 0,
// READ_BLK_MISALIGN[5] = 0, DSR_IMP[4] = X, RESERVERED[3:0] = 0;
//
#define RW_PRTL_MA_DSR_RSRVD_CHK_SDHC(X)   (((X) == 0) || ((X) == 0x10))

//
// READ_BLK_PARTIAL[7] = 1, WRITE_BLK_MISALIGN[6] = X,
// READ_BLK_MISALIGN[5]  = X, DSR_IMP[4] = X, RESERVERED[3:2] = 0
// 2 Highest bits of C_SIZE[1:0]
//
#define RW_PRTL_MA_DSR_RSRVD_CHK_SDSC(X)   (((X) == 0) || ((X) == 0x10))

#define C_SIZE_HI_MASK_SDHC          0x3F
#define C_SIZE_HI_MASK_SDSC          0x03

#define C_SIZE_MULT_MASK             0x03

#endif//SD_SPI_REGS_H
