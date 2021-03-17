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

//
// The CSD-related macros below are to be used to parse and verify the byte 
// contents of the CSD register. They do not necessarily represent the expected
// values. There is a lot of byte overlap in the register contents. 
//

//
// CSD STRUCTURE 
// 
// This value is unique to the card type. Its value is represented by the two
// MSBits in the first byte of the CSD returned. The lower 6 bits of the byte
// are reserved and do not matter (they should = 0).
//
#define CSD_VSN_SDSC        0x00
#define CSD_VSN_SDHC        0x01
//#define CSD_STRUCT_V_SDUC 0x10            // ultra capacity not used here.
#define CSD_STRUCT_MASK     0x40            // extracts the CSD Struct bits.

//
// Determines the CSD Struct Version from the first byte of the CSD register
// and returns the value of the appropriate CSD_STRUCT_V_XXXX macro above.
// 
#define CSD_VSN_CALC(CSD_BYTE)    (((CSD_BYTE) & CSD_STRUCT_MASK)             \
                                    ? CSD_VSN_SDHC : CSD_VSN_SDSC)           

//
// TAAC
//
// Byte 2 of the CSD register is the TAAC. For SDSC type cards, bit 7 of TAAC 
// is reserved, so must be 0. All others could be 1 or 0 and so only bit 7 is
// used to verify the TAAC byte. For SDHC type cards it must be 0x0E (1ms).
//
#define TAAC_SDHC                  0x0E     // SDHC - fixed at 0x0E (1ms)
// TAAC check for SDSC type cards. Returns true if TAAC and false otherwise.
#define TAAC_RSVD_BIT_SDSC         0x80
#define TAAC_CHK_SDSC(TAAC_BYTE)   !((TAAC_BYTE) & TAAC_RSVD_BIT_SDSC)

//
// NSAC - Byte 3 of CSD register
//
//#define NSAC_SDSC         NA             // could be any value.
#define NSAC_SDHC           0              // Not used. Field is present

//
// TRANS SPEED - Byte 4 of the CSD register.
//
#define TRANS_SPEED         0x32           // Default val.

// 
// CCC
//
// 12 bits beginning at byte 5 through the upper 4 bits of the byte 6. For
// SDSC, CCC has form 01_1101101_1, and for SDHC it has form _1_1101101_1.
//

//
// Pass the high CCC byte (byte 5 of CSD) to the appropriate checker below, 
// depending on card type. The checker will return true if it is valid.
//
#define CCC_HI_BYTE_CHK_SDSC(CCC_HI_BYTE)  ((CCC_HI_BYTE) | 0x40) == 0x7B
#define CCC_HI_BYTE_CHK_SDHC(CCC_HI_BYTE)  ((CCC_HI_BYTE) | 0xA0) == 0xFB

//
// The lower 4 bits of CCC are the upper 4 bits of byte 6 of the CSD. This mask
// can be used to test/extract these from byte 6.
//
#define CCC_LO_MASK     0x50                // same for SDSC and SDHC

//
// READ_BL_LEN - The lower 4 bits of byte 6 of the CSD.
//
#define RBL_MASK        0x0F                // to extract RBL
#define RBL_SDHC        0x09                // must be 9 for SDHC
#define RBL_LO_SDSC     0x09                // RBL for SDSC is 9, 10, or 11
#define RBL_HI_SDSC     0x0B

// 
// BYTE 7 of CSD includes the listed single bit fields (with values).
//

//
// For SDSC, this checker verifies expected value of CSD byte 7:
//
// READ_BLK_PARTIAL[7] = 1, 
// WRITE_BLK_MISALIGN[6] = X,
// READ_BLK_MISALIGN[5] = X, 
// DSR_IMP[4] = X, 
// RESERVED[3:2] = 0
// 2 Highest bits of C_SIZE[1:0]
//
#define RP_WBM_RBM_DSR_RSRVD_CHK_SDSC(MBF)   (((MBF) & 0x8C) == 0x80)

//
// For SDHC, this checker verifies expected value of CSD byte 7:
//
// READ_BLK_PARTIAL[7] = 0, 
// WRITE_BLK_MISALIGN[6] = 0,
// READ_BLK_MISALIGN[5]  = 0, 
// DSR_IMP[4] = X, 
// RESERVED[3:0] = 0;
//
#define RP_WBM_RBM_DSR_RSRVD_CHK_SDHC(MBF)   (((MBF) == 0) || ((MBF) == 0x10))

//
// C_SIZE - byte span in CSD is dependent on card type.
//

//
// for SDSC, C_SIZE is 12 bits and the upper 2 bits of the field are the lowest
// two bits of byte 7. Use this mask to extract these bits from CSD byte 7.
//
#define C_SIZE_HI_MASK_SDSC     0x03

//
// for SDHC, C_SIZE is 22 bits and the upper 6 bits of the field are, are the
// lower 6 bits of byte 8. The upper 2 bits of byte 8 are reserved (0). This
// mask is used to extract these upper bits.
//
#define C_SIZE_HI_MASK_SDHC     0x3F

//
// C_SIZE_MULT
//
// Only used for SDSC. The field is 3 bits, but spans two CSD bytes. This mask
// is used to extract the upper two bits of the field from CSD byte 10. The
// lowest field bit is the MSBit of CSD byte 11.
//
#define C_SIZE_MULT_HI_MASK     0x03

#endif //SD_SPI_REGS_H
