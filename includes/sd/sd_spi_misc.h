/*
 * File    : SD_SPI_MISC.H
 * Version : 1.0
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : MIT
 * Copyright (c) 2020, 2021
 *
 * This is meant to be a catch-all for some misellaneous functions. Will 
 * require SD_SPI_BASE and SD_SPI_RWE.
 */

#ifndef SD_SPI_MISC_H
#define SD_SPI_MISC_H

/*
 ******************************************************************************
 *                               GENERAL  MACROS
 ******************************************************************************
 */
#define FAILED_CAPACITY_CALC     1     // failed memory capacity calculation
#define START_BLOCK_TKN_MBW      0xFC  // multi-block write start block token
#define STOP_TRANSMIT_TKN_MBW    0xFD  // multi-block write data TX stop

//
// used by sd_FindNonZeroDataBlockNums to specify the number of data block
// numbers/addresses to print per line.
//
#define NZDBN_PER_LINE           5

/*
 ******************************************************************************
 *                      CARD-SPECIFIC DATA REGISTER (CSD) MACROS
 ******************************************************************************
 */

//
// The macros in this section are currently only used for the memory capacity
// functions and are split up into CSD Version 1 (SDSC) macros and 
// CSD Version 2 (SDHC/SDXC) macros. 
//
// These CSD macros are used to parse the 128 bit CSD register into its fields
// and verify the fields where possible. In SPI mode, these 128 bits are sent 
// in 8-bit byte packets, with many fields spanning multiple bytes and/or
// sharing bytes. As such, many of the macros do not necessarily equal their 
// respective field values, but rather values related to the placement of 
// all or partial fields within a byte.
// 
// Notes: 1) many of the CSD bits are reserved (not used). 
//        2) when referring to the byte numbers below, these numbers are the 
//           order that the CSD bytes are received by the host from the SD 
//           card, though in actuality, they being sent MSB (bit 127) --> LSB
//           (bit 0).
//  
 
//
// CSD STRUCTURE
// 
// This value is unique to the card type. Its value is represented by the two
// MSBits in the first byte of the CSD returned. The lower 6 bits of the byte
// are reserved and do not matter (they should = 0).
//
#define CSD_VSN_SDSC        0x00            // version 1 type
#define CSD_VSN_SDHC        0x01            // version 2 type 
//#define CSD_STRUCT_V_SDUC 0x10            // version 3 type. not used.

//
// Determines the CSD Struct Version from the first byte of the CSD register
// and returns the value of the appropriate CSD_STRUCT_V_XXXX macro above.
// 
#define CSD_STRUCT_MASK           0x40      // extracts the CSD Struct bits.
#define GET_CSD_VSN(CSD_BYTE)     (((CSD_BYTE) & CSD_STRUCT_MASK)             \
                                  ? CSD_VSN_SDHC : CSD_VSN_SDSC)

/*
 ------------------------------------------------------------------------------
 *                                                  CSD VERSION 1 (SDSC) MACROS
 * 
 * Description : Macros to be used with CSD Version 1 (SDSC) type cards.
 ------------------------------------------------------------------------------
 */

//
// TAAC for SDSC (8b) - byte 2 of CSD register.
//
// Bit 7 of the TAAC in SDSC types is reserved, it must be 0. All others bits 
// could be 1 or 0. As such bit 7 is used as a weak check to verify the TAAC 
// byte. The TAAC checker will return true if bit 7 is 0 (assume TAAC) and 
// false otherwise.
//
#define TAAC_RSVD_BIT_SDSC         0x80
#define TAAC_CHK_SDSC(TAAC_BYTE)   !((TAAC_BYTE) & TAAC_RSVD_BIT_SDSC)

//
// NSAC for SDSC (8b) - Byte 3 of CSD register. Could be any value and it is
//                      not used in this current implementation.
//
//#define NSAC_SDSC         NA

//
// TRANS SPEED for SDSC (8b) - Byte 4 of the CSD register. 0x32 is the default
//                             value and the only one that will work here.
//                             
#define TRANS_SPEED_SDSC    0x32

// 
// CCC for SDSC (12b) - Occupies all of CSD byte 5 and the upper 4 bits of 
//                      byte 6. The form of the CCC field for SDSC is 
//                      01_1101101_1, where the '_' indicates either 1 or 0.
//

//
// Pass the high CCC byte of SDSC types (byte 5 of CSD) to the checker below. 
// The checker will test that this byte has a valid pattern by converting the
// '_' bit to 1 and then testing that this is equal to 0x7B. The checker will 
// return true (1) if it is valid, and false (0) if not. 
//
#define CCC_HI_BYTE_CHK_SDSC(CCC_HI_BYTE)  ((CCC_HI_BYTE) | 0x40) == 0x7B

//
// The lower 4 bits of CCC are the upper 4 bits of byte 6 of the CSD. This mask
// can be used to test/extract these from byte 6.
//
#define CCC_LO_BITS_MASK_SDSC     0x50      // same for SDSC and SDHC

//
// READ_BL_LEN (4b) - The lower 4 bits of byte 6 of the CSD. These must be
//                    either 9, 10, or 11.
//
#define RBL_MASK_SDSC   0x0F                // extracts RBL from low CCC bits
#define RBL_LO_SDSC     0x09                
#define RBL_HI_SDSC     0x0B

// 
// BYTE 7 of CSD in SDSC - structured as below, with several Single Bit Felds,
//                         and the two highest bits of the C_SIZE field.
//
// READ_BLK_PARTIAL[7] = 1, 
// WRITE_BLK_MISALIGN[6] = X,
// READ_BLK_MISALIGN[5] = X, 
// DSR_IMP[4] = X, 
// RESERVED[3:2] = 0
// 2 Highest bits of C_SIZE[1:0]
//

// This checker returns true if pattern of SBF matches that expected in byte 7
#define CSD_BYTE_7_CHK_SDSC(SBF)     ((SBF) & 0x8C) == 0x80

//
// C_SIZE for SDSC (12b) - This field spans 3 CSD bytes (2b:8b:2b). There is no
//                         specific pattern that can be used to verify the 
//                         field. The upper 2 bits of the field are the lowest 
//                         two bits of byte 7 above. The lowest two bits of the
//                         field are the upper 2 bits of byte 9.
//
#define C_SIZE_HI_MASK_SDSC     0x03        // extract lower 2 bits of byte 7
#define C_SIZE_LO_MASK_SDSC     0xC0        // extract upper 2 bits of byte 9


//
// Between the C_SIZE and C_SIZE_MULT fields are the R/W min/max current
// fields. These are not currently used in this implementation.
//

//
// C_SIZE_MULT for SDSC (3b) - This field spans bytes 10 and 11 of the CSD. The
//                             upper 2 bits of this field are the lower 2 bits
//                             of byte 10, and the lowest byte of this field is
//                             bit 7 of byte 11.
//
#define C_SIZE_MULT_HI_MASK_SDSC     0x03   // extract lower 2 bits of byte 10
#define C_SIZE_MULT_LO_MASK_SDSC     0x80   // extract highest bit of byte 11

//
// There are other fields beyond C_SIZE_MULT but they are not currently used in 
// this implementation.
//

/*
 ------------------------------------------------------------------------------
 *                                                  CSD VERSION 2 (SDHC) MACROS
 * 
 * Description : Macros to be used with CSD Version 2 (SDHC) type cards.
 ------------------------------------------------------------------------------
 */

//
// TAAC for SDHC (8b) - byte 2 of CSD register.
//
#define TAAC_SDHC     0x0E                  // SDHC - fixed at 0x0E (1ms)

//
// NSAC for SDHC (8b) - byte 3 of CSD register.
//
#define NSAC_SDHC     0                     // Not used, but field is present.

//
// TRANS SPEED for SDHC (8b) - Byte 4 of the CSD register. Same default value
//                             as SDSC type.
//
#define TRANS_SPEED_SDHC    TRANS_SPEED_SDSC

//
// CCC for SDHC (12b) - Occupies all of CSD byte 5 and the upper 4 bits of
//                      byte 6. The form of the CCC field for SDHC is 
//                      _1_1101101_1, where the '_' indicates either 1 or 0.

//
// Pass the high byte (byte 5 of CSD) to the checker, and it will test that 
// this byte has a valid pattern by converting the '_' bit to 1's and then 
// testing the result is 0xFB. The checker will return true (1) if it is valid,
// and false (0) if not. 
//
#define CCC_HI_BYTE_CHK_SDHC(CCC_HI_BITS)     ((CCC_HI_BITS) | 0xA0) == 0xFB

//
// The lower 4 bits of CCC are the upper 4 bits of byte 6 of the CSD. This mask
// can be used to test/extract these from byte 6.
//
#define CCC_LO_BITS_MASK_SDHC     CCC_LO_BITS_MASK_SDSC

//
// READ_BL_LEN - The lower 4 bits of byte 6 of the CSD.
//
#define RBL_MASK_SDHC   RBL_MASK_SDSC       // to extract RBL
#define RBL_SDHC        0x09                // must be 9 for SDHC

// 
// BYTE 7 of CSD in SDHC - structured as below, with several Single Bit Felds.
//
// READ_BLK_PARTIAL[7] = 0, 
// WRITE_BLK_MISALIGN[6] = 0,
// READ_BLK_MISALIGN[5]  = 0, 
// DSR_IMP[4] = X, 
// RESERVED[3:0] = 0;
//

// For SDHC, this checker verifies expected pattern of CSD byte 7:
#define CSD_BYTE_7_CHK_SDHC(SBF)     (SBF) == 0 || (SBF) == 0x10

//
// C_SIZE for SDHC (22b) - This field spans 3 CSD bytes (6b:8b:8b). There is no
//                         specific pattern that can be used to verify the 
//                         field. The upper 6 bits of the field are the lowest 
//                         6 bits of byte 8 above. The HI_MASK is used to 
//                         extract these bits from byte 8.
//
#define C_SIZE_HI_MASK_SDHC     0x3F

//
// There are other fields beyond C_SIZE in SDHC CSD but they are not currently used in 
// this implementation.
//

// memory capacity in bytes. See SD card standard for calc formula.
#define CAPACITY_CALC_SDHC(CSZ)   (((CSZ) + 1) * 512000
/*
 ******************************************************************************
 *                                 FUNCTIONS   
 ******************************************************************************
 */

/* 
 * ----------------------------------------------------------------------------
 *                                                    FIND NON-ZERO DATA BLOCKS
 *                                        
 * Description : Gets the total byte capacity of the SD card and returns the 
 *               value. This function actually just operates to determine the
 *               card type (SDHC or SDSC) and uses this to call the correct 
 *               "private" function that will act to get the appropriate
 *               parameters from the card used and use these to calculate the 
 *               capacity. Which parameters and how the calculation is 
 *               performed is unique to the card type. 
 * 
 * Arguments   : ctv   - ptr to a CTV struct instance that. The value of the 
 *                       'type' member is used to determine which function to 
 *                       call to calculate the byte capacity.
 *
 * Returns     : The byte capacity of the SD card. If FAILED_CAPACITY_CALC (1)
 *               is returned instead, then the calc failed. This is a generic, 
 *               non-descriptive error and is used simply to indicate that an 
 *               issue was encountered during the process of getting the 
 *               capacity - this could include unknown card type, R1 error, 
 *               issue getting register contents, or something else.
 * ----------------------------------------------------------------------------
 */
uint32_t sd_GetCardByteCapacity(const CTV *ctv);

/* 
 * ----------------------------------------------------------------------------
 *                                                    FIND NON-ZERO DATA BLOCKS
 *                                        
 * Description : Search consecutive blocks, from startBlckAddr to endBlckAddr,
 *               inclusive, for those that contain any non-zero values and 
 *               print these block numbers / addresses to the screen.
 * 
 * Arguments   : startBlckAddr   - Address of the first block to search.
 *               endBlckAddr     - Address of the last block to search.
 *
 * Notes       : 1) Useful for finding which blocks may contain raw data.
 *               2) Not fast, so if using, suggest only searching over a small
 *                  range at a time.
 * ----------------------------------------------------------------------------
 */
void sd_FindNonZeroDataBlockNums(const uint32_t startBlckAddr, 
                                 const uint32_t endBlckAddr);

/* 
 * ----------------------------------------------------------------------------
 *                                                        PRINT MULTIPLE BLOCKS
 *           
 * Description : Prints the contents of multiple blocks by calling the 
 *               READ_MULTIPLE_BLOCK SD card command. Each block read in will
 *               be printed to the screen using sd_PrintSingleBlock() function
 *               from SD_SPI_RWE.
 * 
 * Arguments   : startBlckAddr   - Address of the first block to be printed.
 *               numOfBlcks      - The number of blocks to be printed to the 
 *                                 screen starting at startBlckAddr.
 * 
 * Returns     : Read Block Error (upper byte) and R1 Response (lower byte).
 * ----------------------------------------------------------------------------
 */
uint16_t sd_PrintMultipleBlocks(const uint32_t startBlckAddr, 
                                const uint32_t numOfBlcks);

/* 
 * ----------------------------------------------------------------------------
 *                                                        WRITE MULTIPLE BLOCKS
 *           
 * Description : Write the contents of a byte array of length BLOCK_LEN to 
 *               multiple blocks of the SD card. The entire array will be
 *               copied to each block. This function is mostly useful for 
 *               testing the WRITE_MULTIPLE_BLOCK SD card command.
 * 
 * Arguments   : startBlckAddr   - Address of the first block to be written.
 *               numOfBlcks      - Number of blocks to be written to.
 *               dataArr         - Pointer to an array holding the data 
 *                                 contents that will be written to each of 
 *                                 the specified SD card blocks. The array 
 *                                 must be of length BLOCK_LEN.
 * 
 * Returns     : Write Block Error (upper byte) and R1 Response (lower byte).
 * ----------------------------------------------------------------------------
 */
uint16_t sd_WriteMultipleBlocks(const uint32_t startBlckAddr, 
                                const uint32_t numOfBlcks, 
                                const uint8_t *dataArr);

/* 
 * ----------------------------------------------------------------------------
 *                                        GET THE NUMBER OF WELL-WRITTEN BLOCKS
 *           
 * Description : This function sends the SD card command SEND_NUM_WR_BLOCKS. 
 *               This should be called after a failure on a 
 *               WRITE_MULTIPLE_BLOCK command and the Write Error Token is 
 *               returned by the SD Card. This will provide the number of 
 *               blocks that were successfully written before the error 
 *               occurred.
 * 
 * Arguments   : wellWrtnBlcks     Pointer to a value that will be updated by
 *                                 by this function to specify the number of
 *                                 blocks successfully written to before the 
 *                                 error occurred.
 * 
 * Returns     : Read Block Error (upper byte) and R1 Response (lower byte).
 * ----------------------------------------------------------------------------
 */
uint16_t sd_GetNumOfWellWrittenBlocks(uint32_t* wellWrittenBlocks);

#endif // SD_SPI_MISC_H
