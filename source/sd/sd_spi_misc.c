/*
 * File       : SD_SPI_MISC.C
 * Version    : 1.0 
 * Target     : ATMega1280
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020, 2021
 * 
 * Implementation of SD_SPI_MISC.H
 */

#include <stdint.h>
#include "prints.h"
#include "avr_spi.h"
#include "sd_spi_base.h"
#include "sd_spi_rwe.h"
#include "sd_spi_misc.h"

/*
 ******************************************************************************
 *                         "Private" FUNCTION PROTOTYPES  
 ******************************************************************************
 */
static uint32_t pvt_GetByteCapacitySDHC(void);
static uint32_t pvt_GetByteCapacitySDSC(void);

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
uint32_t sd_GetCardByteCapacity(const CTV *ctv)
{
  if (ctv->type == SDHC)
    return pvt_GetByteCapacitySDHC();
  else if (ctv->type == SDSC)
    return pvt_GetByteCapacitySDSC();
  else
    return FAILED_CAPACITY_CALC;
}

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
void sd_FindNonZeroDataBlockNums(uint32_t startBlckAddr, uint32_t endBlckAddr)
{
  // keeps track of numbers printed on each line
  uint16_t numPerLine = 0;        
  
  // loop from start to end block
  for (uint32_t blckNum = startBlckAddr; blckNum <= endBlckAddr; ++blckNum)
  {
    uint8_t blkArr[BLOCK_LEN];
    sd_ReadSingleBlock(blckNum, blkArr);    // load block into blkArr[]
    
    //
    // loop through bytes in blkArr[]. If any are non-zero then print the
    // number that block and break to get the next block.
    //
    for (uint16_t byteNum = 0; byteNum < BLOCK_LEN; ++byteNum)
    {
      if (blkArr[byteNum] != 0)
      {
        // begin new line if numPerLine is multiple of NZDBN_PER_LINE.
        if (!(numPerLine % NZDBN_PER_LINE))
          print_Str("\n\r");
        print_Str("\t\t"); 
        print_Dec(blckNum);
        ++numPerLine;
        break;
      }
    }
  }
}

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
uint16_t sd_PrintMultipleBlocks(uint32_t startBlckAddr, uint32_t numOfBlcks)
{
  //
  // send request for SD card to return contents of the blocks starting at the
  // startBlckAddr. If request accepted then R1 resp is OUT_OF_IDLE.
  //
  CS_SD_LOW;
  sd_SendCommand(READ_MULTIPLE_BLOCK, startBlckAddr);
  uint8_t r1 = sd_GetR1();
  if (r1 != OUT_OF_IDLE)
  {
    CS_SD_HIGH;
    return (R1_ERROR | r1);
  }

  //
  // This loop will read in the next block and print its contents to the screen
  // on each loop iteration.
  //
  for (uint8_t blckNum = 0; blckNum < numOfBlcks; ++blckNum)
  {
    uint8_t  blckArr[BLOCK_LEN];

    // print the block number for the current iteration.
    print_Str("\n\n\r                                    BLOCK ");
    print_Dec(startBlckAddr + blckNum);
    
    // loop until Start Block Token received. Return error if timeout.
    for (uint16_t timeout = 0; sd_ReceiveByteSPI() != START_BLOCK_TKN;)
      if (++timeout > TIMEOUT_LIMIT) 
        return (START_TOKEN_TIMEOUT | r1);

    // Load array with data from SD card block.
    for (uint16_t byteNum = 0; byteNum < BLOCK_LEN; ++byteNum) 
      blckArr[byteNum] = sd_ReceiveByteSPI();
    
    // 16-bit CRC. CRC is off (default) so values returned do not matter.
    sd_ReceiveByteSPI(); 
    sd_ReceiveByteSPI();

    // print the block to the screen.
    sd_PrintSingleBlock(blckArr);
  }
  
  // stop SD card from sending data blocks
  sd_SendCommand(STOP_TRANSMISSION, 0);
  
  // R1b response. Don't care.
  sd_ReceiveByteSPI(); 

  CS_SD_HIGH;
  return READ_SUCCESS;
}

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
uint16_t sd_WriteMultipleBlocks(uint32_t startBlckAddr, uint32_t numOfBlcks, 
                                const uint8_t dataArr[])
{
  uint16_t retTkn = INVALID_DATA_RESPONSE;  // initialize return value

  //
  // send request to write to multiple blocks on the SD card beginning at the 
  // startBlckAddr. If accepted, this will continue until the Stop Transmission
  // byte token is sent. This is not the STOP_TRANSMISSION SD card command.
  //
  CS_SD_LOW;    
  sd_SendCommand(WRITE_MULTIPLE_BLOCK, startBlckAddr);
  uint8_t r1 = sd_GetR1();
  if (r1 != OUT_OF_IDLE)
  {
    CS_SD_HIGH;
    return (R1_ERROR | r1);
  }

  // loop over the blocks beginning with block at startBlckAddr
  for (uint32_t blckNum = 0; blckNum < numOfBlcks; ++blckNum)
  {
    uint8_t dataRespTkn = 0;
    
    // send the multi-block write Start Block Token to initiate data transfer
    sd_SendByteSPI(START_BLOCK_TKN_MBW); 

    // send data for a single block to SD card, 1 byte at a time.
    for (uint16_t byteNum = 0; byteNum < BLOCK_LEN; ++byteNum)
      sd_SendByteSPI(dataArr[byteNum]);

    // Send 16-bit CRC. Off by default, so values do not matter.
    sd_SendByteSPI(0xFF);
    sd_SendByteSPI(0xFF);
  
    // loop until valid data response token received or function exits on timeout
    for (uint16_t timeout = 0; 
         dataRespTkn != DATA_ACCEPTED_TKN
         && dataRespTkn != CRC_ERROR_TKN 
         && dataRespTkn != WRITE_ERROR_TKN;)
    {
      dataRespTkn = sd_ReceiveByteSPI() & DATA_RESPONSE_TKN_MASK;
      if (++timeout > TIMEOUT_LIMIT)
      {
        CS_SD_HIGH;
        return (DATA_RESPONSE_TIMEOUT | r1);
      }
    }

    //
    // if SD card signals the data was accepted by returning the Data Accepted
    // Token then the card will enter 'busy' state while it writes data to the
    // block. While busy, the card will hold the DO line at 0. If the timeout
    // limit is reached then the function will return CARD_BUSY_TIMEOUT. If at
    // any block write a CRC ERROR or WRITE ERROR TOKEN are received then the
    // loop should break at which point the function sends the stop transmit.
    //
    if (dataRespTkn == DATA_ACCEPTED_TKN)     
    {
      for (uint16_t timeout = 0; sd_ReceiveByteSPI() == 0; ++timeout)
        if (timeout > 2 * TIMEOUT_LIMIT)    // increased timeout limit
        {
          CS_SD_HIGH;
          return (CARD_BUSY_TIMEOUT | r1);
        }
      retTkn = DATA_WRITE_SUCCESS;
    }
    else if (dataRespTkn == CRC_ERROR_TKN)
    {
      retTkn = CRC_ERROR_TKN_RECEIVED;
      break;
    }
    else if (dataRespTkn == WRITE_ERROR_TKN) 
    {
      retTkn = WRITE_ERROR_TKN_RECEIVED;
      break;
    }
  }

  // Stop Transmission. 0xFD is the Stop Transmission Token
  sd_SendByteSPI(STOP_TRANSMIT_TKN_MBW);
  for (uint16_t timeout = 0; sd_ReceiveByteSPI() == 0; ++timeout)
    if (timeout > 2 * TIMEOUT_LIMIT)        // increased timeout limit
    {
      CS_SD_HIGH;
      return (CARD_BUSY_TIMEOUT | r1);
    }

  //
  // Have found that even after CARD_BUSY is no longer true, that if another
  // command is immediately issued, it results in errors unless some delay is
  // added before de-asserting CS here.
  //
  sd_WaitSendDummySPI(0x5FF);
  CS_SD_HIGH;

  return (retTkn | r1);                            // successful write.
}

/* 
 * ----------------------------------------------------------------------------
 *                                        GET THE NUMBER OF WELL-WRITTEN BLOCKS
 *           
 * Description : This function sends the SD card command SEND_NUM_WR_BLOCKS. 
 *               This should be called after a failure on a 
 *               WRITE_MULTIPLE_BLOCK command and the Write Error Token is 
 *               returned by the SD Card. Issuing this command will request
 *               that the number of blocks that were successfully written to 
 *               before the error occurred be returned.
 * 
 * Arguments   : wellWrtnBlcks   - Pointer to a value that will be updated by
 *                                 by this function to specify the number of
 *                                 blocks successfully written to before a  
 *                                 write error occurred on multi-block write.
 * 
 * Returns     : Read Block Error (upper byte) and R1 Response (lower byte).
 * 
 * Warning     : This function has not been tested yet.
 * ----------------------------------------------------------------------------
 */
uint16_t sd_GetNumOfWellWrittenBlocks(uint32_t *wellWrtnBlcks)
{
  uint8_t  r1;

  // Send APP_CMD to signal next command is an ACMD type command
  CS_SD_LOW;
  sd_SendCommand(APP_CMD, 0);                             
  if ((r1 = sd_GetR1()) != OUT_OF_IDLE) 
  {   
    CS_SD_HIGH;
    return (R1_ERROR | r1);
  }

  // Get number of well written blocks
  sd_SendCommand(SEND_NUM_WR_BLOCKS, 0);
  if ((r1 = sd_GetR1()) != OUT_OF_IDLE) 
  {
    CS_SD_HIGH;
    return (R1_ERROR | r1);
  }

  // loop until Start Block Token received. Return error if timeout.
  for (uint16_t timeout = 0; sd_ReceiveByteSPI() != START_BLOCK_TKN;)
    if (++timeout > TIMEOUT_LIMIT) 
      return (START_TOKEN_TIMEOUT | r1);
  
  // Get the number of well written blocks (32-bit)
  *wellWrtnBlcks  = sd_ReceiveByteSPI();
  *wellWrtnBlcks <<= 8;
  *wellWrtnBlcks |= sd_ReceiveByteSPI();
  *wellWrtnBlcks <<= 8;
  *wellWrtnBlcks |= sd_ReceiveByteSPI();
  *wellWrtnBlcks <<= 8;
  *wellWrtnBlcks |= sd_ReceiveByteSPI();

  // get 16-bit CRC bytes. CRC is off by default so values do not matter.
  sd_ReceiveByteSPI();
  sd_ReceiveByteSPI();

  CS_SD_HIGH;

  return READ_SUCCESS;
}


/*
 ******************************************************************************
 *                         "Private" FUNCTION PROTOTYPES  
 ******************************************************************************
 */

/* 
 * ----------------------------------------------------------------------------
 *                                  (PRIVATE) GET MEMORY CAPACITY FOR TYPE SDSC 
 * 
 * Description : This function will get the parameters from an SDSC type SD 
 *               card that are needed to calculate the card's capacity and use
 *               these to calculate and return the card's byte capacity.
 * 
 * Arguments   : void
 * 
 * Returns     : card capacity in bytes, or FAILED_CAPACITY_CALC
 * ---------------------------------------------------------------------------
 */
static uint32_t pvt_GetByteCapacitySDSC(void)
{
  // CSD fields used to calculate memory capacity
  uint8_t  readBlkLen = 0;
  uint16_t cSize = 0;
  uint8_t  cSizeMult = 0;

  //
  // SEND_CSD (CMD9) to get the CSD register contents. The CSD register
  // contains the necessary parameters required to calculate the card's cap.
  //
  CS_SD_LOW;
  sd_SendCommand(SEND_CSD, 0);
  if (sd_GetR1() != OUT_OF_IDLE) 
  { 
    CS_SD_HIGH; 
    return FAILED_CAPACITY_CALC; 
  }

  //
  // CSD_STRUCTURE
  //
  for (uint16_t timeout = 0; 
       GET_CSD_VSN(sd_ReceiveByteSPI()) != CSD_VSN_SDSC;)
    if (++timeout >= TIMEOUT_LIMIT)
    { 
      CS_SD_HIGH;
      return FAILED_CAPACITY_CALC; 
    }

  //
  // TAAC
  //
  for (uint16_t timeout = 0; !TAAC_CHK_SDSC(sd_ReceiveByteSPI());)
    if (++timeout >= TIMEOUT_LIMIT)
    { 
      CS_SD_HIGH;
      return FAILED_CAPACITY_CALC; 
    }

  //
  // NSAC - Any value is valid for SDSC
  //
  sd_ReceiveByteSPI();

  //
  // TRAN_SPEED - this tests for default value.
  //
  for (uint16_t timeout = 0; sd_ReceiveByteSPI() != TRANS_SPEED_SDSC;)
    if (++timeout >= TIMEOUT_LIMIT)
    { 
      CS_SD_HIGH;
      return FAILED_CAPACITY_CALC; 
    }

  //
  // CCC and READ_BL_LEN
  //

  // Next 2 bytes contain the 12-bit CCC and 4-bit READ_BL_LEN.
  for (uint16_t timeout = 0;; ++timeout)
  {
    if (CCC_HI_BYTE_CHK_SDSC(sd_ReceiveByteSPI()))
    {
      // value required for capacity calculation
      readBlkLen = sd_ReceiveByteSPI() & RBL_MASK_SDSC;
      if (readBlkLen >= RBL_LO_SDSC || readBlkLen <= RBL_HI_SDSC)
        break;
      else
      {
        CS_SD_HIGH;
        return FAILED_CAPACITY_CALC;
      }
    }
    if (timeout >= TIMEOUT_LIMIT)
    { 
      CS_SD_HIGH;
      return FAILED_CAPACITY_CALC;
    }
  }

  //
  // C_SIZE and C_SIZE_MULT are remaining fields needed to calculate capacity
  // 
  for (uint16_t timeout = 0;; ++timeout)
  {
    // get byte containing 2 highest bits of cSize
    cSize = sd_ReceiveByteSPI() & C_SIZE_HI_MASK_SDSC; 
    if (cSize == C_SIZE_HI_MASK_SDSC)
    {                   
      // position and load rest of cSize bits
      cSize <<= 8;           
      cSize |= sd_ReceiveByteSPI();
      cSize <<= 2;        
      cSize |= sd_ReceiveByteSPI() >> 6;
      
      // postion and load cSizeMult bits
      cSizeMult  = (sd_ReceiveByteSPI() & C_SIZE_MULT_HI_MASK_SDSC) << 1;
      cSizeMult |= sd_ReceiveByteSPI() >> 7;

    }
    if (timeout >= TIMEOUT_LIMIT) 
    {
      CS_SD_HIGH; 
      return FAILED_CAPACITY_CALC; 
    }
  }
  CS_SD_HIGH;

  //
  // Calculate Memory Capacity
  //

  // blockLen = 2^READ_BL_LEN
  uint16_t blockLen = 1;                             
  for (uint8_t pow = 0; pow < readBlkLen; ++pow) 
    blockLen *= 2;

  // mult = 2^(cSizeMult + 2)
  uint16_t mult = 1;
  for (uint8_t pow = 0; pow < cSizeMult + 2; ++pow) 
    mult *= 2;

  // Memory capacity in bytes. See SD card standard for calc formula.
  return ((cSize + 1) * mult * blockLen);
}

/* 
 * ----------------------------------------------------------------------------
 *                                  (PRIVATE) GET MEMORY CAPACITY FOR TYPE SDHC 
 * 
 * Description : This function will get the parameters from an SDHC type SD 
 *               card that are needed to calculate the card's capacity and use
 *               these to calculate and return the card's byte capacity.
 * 
 * Arguments   : void
 * 
 * Returns     : card capacity in bytes, or FAILED_CAPACITY_CALC
 * ---------------------------------------------------------------------------
 */
static uint32_t pvt_GetByteCapacitySDHC(void)
{
  // Only C_SIZE is needed to calculate the memory capacity of SDHC types
  uint64_t cSize;

  //
  // SEND_CSD (CMD9) - Request SD card send contents of CSD register
  //
  CS_SD_LOW;
  sd_SendCommand(SEND_CSD, 0);              // arg = 0 for this command
  if (sd_GetR1() != OUT_OF_IDLE)
  {
    CS_SD_HIGH;
    return FAILED_CAPACITY_CALC;
  }
  
  //
  // The CSD register fields accessed first, before C_SIZE, are used here to
  // verfiy CSD is accurately being read-in, and the correct memory capacity
  // function has been called.
  //

  //
  // CSD_STRUCTURE - Must be 1 for SDHC/SDXC types.
  //
  for (uint16_t timeout = 0; 
       GET_CSD_VSN(sd_ReceiveByteSPI()) != CSD_VSN_SDHC;)
    if (++timeout >= TIMEOUT_LIMIT)
    { 
      CS_SD_HIGH;
      return FAILED_CAPACITY_CALC;
    }

  //
  // TAAC - fixed at 1ms (0x0E) for SDHC
  //
  for (uint16_t timeout = 0; sd_ReceiveByteSPI() != TAAC_SDHC;)
    if (++timeout >= TIMEOUT_LIMIT)
    { 
      CS_SD_HIGH;
      return FAILED_CAPACITY_CALC;
    }

  //
  // NSAC - not used for SDHC, but field is still there.
  //
  for (uint16_t timeout = 0; sd_ReceiveByteSPI() != NSAC_SDHC;)
    if (++timeout >= TIMEOUT_LIMIT)
    { 
      CS_SD_HIGH;
      return FAILED_CAPACITY_CALC;
    }

  //
  // TRAN_SPEED
  //
  for (uint16_t timeout = 0; sd_ReceiveByteSPI() != TRANS_SPEED_SDHC;)
    if (++timeout >= TIMEOUT_LIMIT)
    { 
      CS_SD_HIGH;
      return FAILED_CAPACITY_CALC; 
    }

  //
  // CCC and READ_BL_LEN
  //

  // Next 2 bytes contain the 12-bit CCC and 4-bit READ_BL_LEN.
  for (uint16_t timeout = 0;; ++timeout)
  {
    if (CCC_HI_BYTE_CHK_SDHC(sd_ReceiveByteSPI()))
    {
      if (sd_ReceiveByteSPI() != CCC_LO_BITS_MASK_SDHC + RBL_SDHC)
      {
        CS_SD_HIGH; 
        return FAILED_CAPACITY_CALC;
      }
      break;
    }
    if (timeout >= TIMEOUT_LIMIT)
    { 
      CS_SD_HIGH;
      return FAILED_CAPACITY_CALC;
    }
  }

  //
  // C_SIZE is the remaining field needed to calculate capacity
  // 
  for (uint16_t timeout = 0;; ++timeout)
  {
    // Remaining bits before reaching C_SIZE
    if (CSD_BYTE_7_CHK_SDHC(sd_ReceiveByteSPI()))
    {
      cSize = sd_ReceiveByteSPI() & C_SIZE_HI_MASK_SDHC;// Only [5:0] is C_SIZE
      cSize <<= 8;           
      cSize |= sd_ReceiveByteSPI();
      cSize <<= 8;        
      cSize |= sd_ReceiveByteSPI();
      break;
    }
    if (timeout >= TIMEOUT_LIMIT) 
    { 
      CS_SD_HIGH; 
      return FAILED_CAPACITY_CALC; 
    }
  }
  CS_SD_HIGH;

  // see std for calculation description. (cSize + 1) + 512kB
  return ((cSize + 1) * 512000);
}    
