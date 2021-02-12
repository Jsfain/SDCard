/*
 * File    : SD_SPI_MISC.C
 * Version : 0.0.0.1 
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : MIT
 * Copyright (c) 2020
 * 
 * Implementation of SD_SPI_MISC.H
 */

#include <stdint.h>
#include <avr/io.h>
#include "prints.h"
#include "spi.h"
#include "sd_spi_base.h"
#include "sd_spi_rwe.h"
#include "sd_spi_misc.h"

/*
 ******************************************************************************
 *                                 FUNCTIONS   
 ******************************************************************************
 */

/* 
 * ----------------------------------------------------------------------------
 *                                                          GET MEMORY CAPACITY
 * 
 * Functions to calculate and return the memory capacity of an SD Card in
 * bytes. Which one to use depends on the card type, i.e. SDSC or SDHC. This 
 * should be set in an instance of CTV while intializing the card.
 * ---------------------------------------------------------------------------
 */
uint32_t sd_GetMemoryCapacitySDSC(void)
{
  uint8_t resp;
  uint8_t exitLoopFlag;
  uint8_t timeout;
  // CSD fields used to calculate memory capacity
  uint8_t  readBlckLen = 0;
  uint16_t cSize = 0;
  uint8_t  cSizeMult = 0;
  // additional memory cap calculation variables
  uint16_t blockLen;
  uint16_t mult;

  // SEND_CSD (CMD9)
  CS_SD_LOW;
  sd_SendCommand(SEND_CSD, 0);
  if (sd_GetR1() != OUT_OF_IDLE) 
  { 
    CS_SD_HIGH; 
    return 1; 
  }

  //
  // CSD_STRUCTURE
  //
  timeout = 0;
  do
  { 
    // CSD structure is bit 6 and must be 0 for SDSC.
    if (!(sd_ReceiveByteSPI() | 0x40)) 
      break;
    if (timeout++ >= TIMEOUT_LIMIT)
    { 
      CS_SD_HIGH;
      return 1; 
    }
  }
  while(1);

  //
  // TAAC
  //
  timeout = 0;
  do
  { 
    // Bit 7 of TAAC is reserved so must be 0
    if (!(sd_ReceiveByteSPI() | 0x80)) 
      break;
    if (timeout++ >= TIMEOUT_LIMIT) 
    { 
      CS_SD_HIGH; 
      return 1;
    }
  }
  while(1);

  //
  // NSAC - Any value is valid
  //
  sd_ReceiveByteSPI();

  //
  // TRAN_SPEED
  //
  timeout = 0;
  do
  { 
    resp = sd_ReceiveByteSPI();
    // TRANS SPEED must be 0x32 or 0x5A to be valid
    if (resp == 0x32 || resp == 0x5A) 
      break;
    if (timeout++ >= TIMEOUT_LIMIT)
    { 
      CS_SD_HIGH; 
      return 1;
    }
  }
  while(1);
 
  //
  // CCC and READ_BL_LEN
  // 
  // Next 2 bytes are the 12-bit CCC and 4-bit READ_BL_LEN. CCC is of the
  // form 01_110110101. READ_BL_LEN is needed to calculate memory capacity.
  // It's value must be 9, 10, or 11.
  //
  timeout = 0;
  exitLoopFlag = 1;
  while (exitLoopFlag == 1)
  {
    // CCC[11:4] - converts _ bit to 1 and test = 0x7B
    if ((sd_ReceiveByteSPI() | 0x40) == 0x7B)
    {
      // CCC[3:0] = 0x5 and READ_BL_LEN is the lower byte
      if ((resp = sd_ReceiveByteSPI()) == 0x50) 
      {
        readBlckLen = resp & 0x0F;
        if (readBlckLen < 9 || readBlckLen > 11) 
        { 
          CS_SD_HIGH;
          return 1;
        }
        exitLoopFlag = 0;
      }
    }
    if (timeout++ >= TIMEOUT_LIMIT)
    { 
      CS_SD_HIGH;
      return 1;
    }
  }

  //
  // C_SIZE and C_SIZE_MULT are remaining fields needed to calculate capacity
  // 
  timeout = 0;
  exitLoopFlag = 1;
  while(exitLoopFlag == 1)
  {
    //
    // READ_BLK_PARTIAL[7] = 1, WRITE_BLK_MISALIGN[6] = X,
    // READ_BLK_MISALIGN[5]  = X, DSR_IMP[4] = X, RESERVERED[3:2] = 0
    //
    if((resp = sd_ReceiveByteSPI()) & 0xF3)
    {           
      cSize = resp & 0x03;         
      cSize <<= 8;           
      cSize |= sd_ReceiveByteSPI();
      cSize <<= 2;        
      cSize |= sd_ReceiveByteSPI() >> 6;
      
      cSizeMult  = (sd_ReceiveByteSPI() & 0x03) << 1;
      cSizeMult |=  sd_ReceiveByteSPI() >> 7;
      
      exitLoopFlag = 0;
    }
    if (timeout++ >= TIMEOUT_LIMIT) 
    {
      CS_SD_HIGH; 
      return 1; 
    }
  }
  CS_SD_HIGH;

  //
  // Calculate Memory Capacity
  //

  // blockLen = 2^READ_BL_LEN
  blockLen = 1;                             
  for (uint8_t pow = 0; pow < readBlckLen; pow++) 
    blockLen = blockLen * 2;

  // mult = 2^(cSizeMult + 2)
  mult = 1;
  for (uint8_t pow = 0; pow < cSizeMult + 2; pow++) 
    mult = mult * 2;

  // Memory capacity in bytes. See SD card standard for calc formula.
  return ((cSize + 1) * mult * blockLen);
}

uint32_t sd_GetMemoryCapacitySDHC(void)
{
  uint8_t  resp;
  uint8_t  exitLoopFlag;
  uint8_t  timeout;
  
  // Only C_SIZE is needed to calculate the memory capacity of SDHC/SDXC types
  uint64_t cSize;

  //
  // SEND_CSD (CMD9)
  //
  CS_SD_LOW;
  sd_SendCommand(SEND_CSD, 0);
  if (sd_GetR1() != OUT_OF_IDLE)
  {
    CS_SD_HIGH;
    return 1;
  }
  
  //
  // The CSD register fields accessed first, before C_SIZE, are used here to
  // verfiy CSD is accurately being read-in, and the correct memory capacity
  // function has been called.
  //

  //
  // CSD_STRUCTURE - Must be 1 for SDHC/SDXC types.
  //
  timeout = 0;
  do
  { 
    // CSD structure is bit 6 and must be 1 for SDHC/SDXC.
    if (sd_ReceiveByteSPI() | 0x40)
      break;
    if (timeout++ >= TIMEOUT_LIMIT)
    { 
      CS_SD_HIGH; 
      return 1; 
    }
  }
  while(1);

  //
  // TAAC
  //
  timeout = 0;
  do
  { 
    // TAAC - Must be 0X0E (1ms)
    if (sd_ReceiveByteSPI() == 0x0E) 
      break;
    if (timeout++ >= TIMEOUT_LIMIT) 
    { 
      CS_SD_HIGH; 
      return 1; 
    }
  }
  while(1);

  //
  // NSAC
  //
  timeout = 0;
  do
  { 
    // NSAC - Must be 0
    if (sd_ReceiveByteSPI() == 0) 
      break;
    if (timeout++ >= TIMEOUT_LIMIT) 
    {
      CS_SD_HIGH; 
      return 1; 
    }
  }
  while(1);

  //
  // TRAN_SPEED
  //
  timeout = 0;
  do
  { 
    // TRAN_SPEED - Must be 0x32 for current implementation
    if (sd_ReceiveByteSPI() == 0x32)
      break;
    if (timeout++ >= 0xFF) 
    { 
      CS_SD_HIGH; 
      return 1; 
    }
  }
  while(1);

  //
  // CCC and READ_BL_LEN
  //
  // Next 2 bytes contain the 12-bit CCC and 4-bit READ_BL_LEN. CCC is of the
  // form _1_1101101_1 for SDHC/SDXC. READ_BL_LEN must be 9 for SDHC/SDXC.
  //
  exitLoopFlag = 1;
  timeout = 0;
  while (exitLoopFlag == 1)
  {
    // CCC[11:4] - converts _ bits to 1 and test = 0xFB
    if ((sd_ReceiveByteSPI() | 0xA0) == 0xFB) 
    {
      // CCC[3:0] = 0x5 and READ_BL_LEN = 0x09
      if (sd_ReceiveByteSPI() != 0x59) 
      {
        CS_SD_HIGH; 
        return 1;
      }
      exitLoopFlag = 0;
    }
    if (timeout++ >= TIMEOUT_LIMIT)
    { 
      CS_SD_HIGH;
      return 1;
    }
  }

  //
  // C_SIZE is the remaining field needed to calculate capacity
  // 
  exitLoopFlag = 1;
  timeout = 0;
  while (exitLoopFlag == 1)
  {
    //
    // Remaining bits before reaching C_SIZE
    // READ_BLK_PARTIAL[7] = 0, WRITE_BLK_MISALIGN[6] = 0,
    // READ_BLK_MISALIGN[5] = 0, DSR_IMP[4] = X, RESERVERED[3:0] = 0;
    //
    resp = sd_ReceiveByteSPI();
    if (resp == 0 || resp == 16) 
    {
      cSize = sd_ReceiveByteSPI() & 0x3F;   // Only [5:0] is C_SIZE          
      cSize <<= 8;           
      cSize |= sd_ReceiveByteSPI();
      cSize <<= 8;        
      cSize |= sd_ReceiveByteSPI();

      exitLoopFlag = 0;
    }
    if (timeout++ >= TIMEOUT_LIMIT) 
    { 
      CS_SD_HIGH; 
      return 1; 
    }
  }
  CS_SD_HIGH;

  // memory capacity in bytes. See SD card standard for calc formula.
  return ((cSize + 1) * 512000);
}

/* 
 * ----------------------------------------------------------------------------
 *                                                    FIND NON-ZERO DATA BLOCKS
 *                                        
 * Description : Search consecutive blocks (from startBlckAddr to endBlckAddr)
 *               for thos that contain any non-zero values and print those 
 *               block numbers/addresses to the screen.
 * 
 * Arguments   : startBlckAddr     Address of the first block to search.
 *               
 *               endBlckAddr       Address of the last block to search.
 *
 * Notes       : 1) Useful for finding which blocks may contain raw data.
 * 
 *               2) Not fast, so suggest only search over a small range.
 * ----------------------------------------------------------------------------
 */
void sd_FindNonZeroDataBlockNums(uint32_t startBlckAddr, uint32_t endBlckAddr)
{
  uint16_t newLine = 0;                     // for formatting output   

  for (uint32_t blckNum = startBlckAddr; blckNum <= endBlckAddr; blckNum++)
  {
    uint8_t  blkArr[BLOCK_LEN];             // data block array
  
    sd_ReadSingleBlock(blckNum, blkArr);       
    
    for (uint16_t byteNum = 0; byteNum < BLOCK_LEN; byteNum++)
    {
      if (blkArr[byteNum] != 0)
      {
        if (newLine % 5 == 0)               // newLine every 5 values output
          print_Str("\n\r");
        print_Str("\t\t"); 
        print_Dec(blckNum);
        newLine++;
        break;
      }
    }
  }
}

/* 
 * ----------------------------------------------------------------------------
 *                                                        PRINT MULTIPLE BLOCKS
 *           
 * Description : Prints multiple blocks by calling the READ_MULTIPLE_BLOCK SD 
 *               card command. Each block read in will be printed to the screen
 *               using sd_PrintSingleBlock() function from SD_SPI_RWE.
 * 
 * Arguments   : startBlckAddr     Address of the first block to be printed.
 *    
 *               numOfBlcks        The number of blocks to be printed to the 
 *                                 screen starting at startBlckAddr.
 * 
 * Returns     : Read Block Error (upper byte) and R1 Response (lower byte).
 * ----------------------------------------------------------------------------
 */
uint16_t sd_PrintMultipleBlocks(uint32_t startBlckAddr, uint32_t numOfBlcks)
{
  uint8_t  r1;                    

  CS_SD_LOW;
  sd_SendCommand(READ_MULTIPLE_BLOCK, startBlckAddr);      // CMD18
  if ((r1 = sd_GetR1()) != OUT_OF_IDLE)
  {
    CS_SD_HIGH;
    return (R1_ERROR | r1);
  }
  else
  {
    for (uint8_t blckNum = 0; blckNum < numOfBlcks; blckNum++)
    {
      uint8_t  blckArr[BLOCK_LEN];         // data block array

      print_Str("\n\r Block ");
      print_Dec(startBlckAddr + blckNum);
      
      //
      // 0xFE is the Start Block Token to be sent by
      // SD Card to signal data is about to be sent.
      //
      uint16_t timeout = 0;
      while (sd_ReceiveByteSPI() != 0xFE) 
      {
        if (timeout++ > 0x0F * TIMEOUT_LIMIT) 
          return (START_TOKEN_TIMEOUT | r1);
      }

      // Load array with data from SD card block.
      for (uint16_t byteNum = 0; byteNum < BLOCK_LEN; byteNum++) 
        blckArr[byteNum] = sd_ReceiveByteSPI();
      
      // 16-bit CRC. Should be off (default) so values do not matter.
      sd_ReceiveByteSPI(); 
      sd_ReceiveByteSPI();

      // print the block to the screen.
      sd_PrintSingleBlock(blckArr);
    }
    
    // stop SD card from sending data blocks
    sd_SendCommand(STOP_TRANSMISSION, 0);
    
    // R1b response. Don't care.
    sd_ReceiveByteSPI(); 
  }
  CS_SD_HIGH;
  
  return READ_SUCCESS;
}

/* 
 * ----------------------------------------------------------------------------
 *                                                        WRITE MULTIPLE BLOCKS
 *           
 * Description : Write the contents of a byte array of length BLOCK_LEN to 
 *               multiple blocks of the SD card. The entire array data will be 
 *               copied to each block. This function is not that useful, but 
 *               used to test/demo the SD card command WRITE_MULTIPLE_BLOCK.
 * 
 * Arguments   : startBlckAddr     Address of the first block to be written.
 * 
 *               numOfBlcks        Number of blocks to be written to.
 * 
 *               dataArr           Pointer to an array holding the data 
 *                                 contents that will be written to each of 
 *                                 the specified SD card blocks. Array must be
 *                                 of length BLOCK_LEN.
 * 
 * Returns     : Write Block Error (upper byte) and R1 Response (lower byte).
 * ----------------------------------------------------------------------------
 */
uint16_t sd_WriteMultipleBlocks(uint32_t startBlckAddr, uint32_t numOfBlcks, 
                                uint8_t* dataArr)
{
  uint8_t  r1;                              // for the R1 response 
  uint16_t retTkn;                          // for the return value

  CS_SD_LOW;    
  sd_SendCommand(WRITE_MULTIPLE_BLOCK, startBlckAddr);    //CMD25
  
  if ((r1 = sd_GetR1()) != OUT_OF_IDLE)
  {
    CS_SD_HIGH
    return (R1_ERROR | r1);
  }
  else
  {
    uint16_t timeout = 0;

    for (uint32_t blckNum = 0; blckNum < numOfBlcks; blckNum++)
    {
      const uint8_t dataTknMask = 0x1F;    // for extracting data reponse token
      uint8_t dataRespTkn;
      
      //
      // 0xFC is multi-block write Start Block Token. 
      // Send this token to initiate data transfer.
      //
      sd_SendByteSPI(0xFC); 

      // send data to SD card.
      for (uint16_t byteNum = 0; byteNum < BLOCK_LEN; byteNum++)
        sd_SendByteSPI(dataArr[byteNum]);

      // Send 16-bit CRC. Off by default, so values do not matter.
      sd_SendByteSPI(0xFF);
      sd_SendByteSPI(0xFF);
      
      // wait for valid data response token
      do
      { 
        dataRespTkn = sd_ReceiveByteSPI();
        if (timeout++ > 2 * TIMEOUT_LIMIT)
        {
          CS_SD_HIGH;
          return (DATA_RESPONSE_TIMEOUT | r1);
        }  
      }
      while ((dataTknMask & dataRespTkn) != 0x05 &&        // DATA_ACCEPTED
             (dataTknMask & dataRespTkn) != 0x0B &&        // CRC_ERROR
             (dataTknMask & dataRespTkn) != 0x0D);         // WRITE_ERROR

      // Data Accepted --> Data Response Token = 0x05 
      if ((dataRespTkn & 0x05) == 0x05)     
      {
        timeout = 0;
        
        //
        // Wait for SD card to finish writing data to the block.
        // Data Out (DO) line held low while card is busy writing to block.
        //
        while (sd_ReceiveByteSPI() == 0)
        {
          if (timeout++ > 2 * TIMEOUT_LIMIT) 
            return (CARD_BUSY_TIMEOUT | r1); 
        };
        retTkn = DATA_ACCEPTED_TOKEN_RECEIVED;
      }
      // CRC Error --> Data Response Token = 0x0B 
      else if ((dataRespTkn & 0x0B) == 0x0B)
      {
        retTkn = CRC_ERROR_TOKEN_RECEIVED;
        break;
      }
      // Write Error --> Data Response Token = 0x0D
      else if ((dataRespTkn & 0x0D) == 0x0D) 
      {
        retTkn = WRITE_ERROR_TOKEN_RECEIVED;
        break;
      }
    }

    // Stop Transmission. 0xFD is the Stop Transmission Token
    sd_SendByteSPI(0xFD);
    
    timeout = 0;
    while (sd_ReceiveByteSPI() == 0)
    {
      if (timeout++ > 2 * TIMEOUT_LIMIT) 
      {
        CS_SD_HIGH;
        return (CARD_BUSY_TIMEOUT | r1);
      }
    }
  }

  //
  // Have found sometimes even after CARD_BUSY is no longer set, that if
  // another command is immediately issued, it results in errors unless some
  // delay is added before de-asserting CS here. That's the reason for the loop
  //
  for (uint8_t k = 0; k < 0xFE; k++)
    sd_SendByteSPI(0xFF);

  CS_SD_HIGH;

  return retTkn;                            // successful write.
}

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
uint16_t sd_GetNumOfWellWrittenBlocks(uint32_t* wellWrtnBlcks)
{
  uint8_t  r1;
  uint16_t timeout = 0; 

  CS_SD_LOW;
  // Send APP_CMD to signal next command is an ACMD type command
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

  //
  // 0xFE is the Start Block Token to be sent by the
  // SD Card to signal that data is about to be sent.
  //
  while (sd_ReceiveByteSPI() != 0xFE)
  {
    if(timeout++ > 2 * TIMEOUT_LIMIT) 
    {
      CS_SD_HIGH;
      return (START_TOKEN_TIMEOUT | r1);
    }
  }
  
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
