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

uint32_t sd_getMemoryCapacitySDSC (void)
{
  uint8_t  r1 = 0;
  uint8_t  resp;
  uint8_t  timeout = 0;
  uint8_t  flag = 0;

  // CSD fields used to calculate memory capacity
  uint8_t  readBlckLen = 0;
  uint16_t cSize = 0;
  uint8_t  cSizeMult = 0;

  // For memory cap calculation routine
  uint16_t blockLen;
  uint16_t mult;

  // SEND_CSD (CMD9)
  CS_SD_LOW;
  sd_sendCommand (SEND_CSD, 0);
  r1 = sd_getR1();
  if (r1 > 0) 
  { 
    CS_SD_HIGH; 
    return 1; 
  }

  // CSD_STRUCTURE - Must be 0 for SDSC types.
  do
  { 
    if (sd_receiveByteSPI() >> 6 == 0) 
      break;
    if (timeout++ >= 0xFF)
    { 
      CS_SD_HIGH;
      return 1; 
    }
  }
  while(1);

  // TAAC - Bit 7 is reserved so must be 0
  timeout = 0;
  do
  { 
    if (sd_receiveByteSPI() >> 7 != 0) 
      break;
    if (timeout++ >= 0xFF) 
    { 
      CS_SD_HIGH; 
      return 1;
    }
  }
  while(1);

  // NSAC - Any value is valid
  sd_receiveByteSPI();

  // TRAN_SPEED
  timeout = 0;
  do
  { 
    resp = sd_receiveByteSPI();  
    if (resp == 0x32 || resp == 0x5A) 
      break;
    if (timeout++ >= 0xFF)
    { 
      CS_SD_HIGH; 
      return 1;
    }
  }
  while(1);

  // Next 2 bytes are the 12-bit CCC and 4-bit READ_BL_LEN. CCC is of the
  // form 01_110110101. READ_BL_LEN is needed to calculate memory capacity.
  // It's value must be 9, 10, or 11.
  flag = 1;
  while (flag == 1)
  {
    // CCC[11:4]
    if (sd_receiveByteSPI() & 0x7B)
    {
      // CCC[3:0] and READ_BL_LEN
      if ((resp = sd_receiveByteSPI()) == 0x50) 
      {
        readBlckLen = resp & 0b00001111;
        if (readBlckLen < 9 || readBlckLen > 11) 
        { 
          CS_SD_HIGH;
          return 1;
        }
        flag = 0;
      }
    }
    if (timeout++ >= 0xFF)
    { 
      CS_SD_HIGH;
      return 1;
    }
  }

  // Get remaining fields needed to calc memory capacity: 
  // C_SIZE and C_SIZE_MULT
  flag = 1;
  timeout = 0;
  while(flag == 1)
  {
    // READ_BLK_PARTIAL[7] = 1, WRITE_BLK_MISALIGN[6] = X,
    // READ_BLK_MISALIGN[5]  = X, DSR_IMP[4] = X, RESERVERED[3:2] = 0
    if((resp = sd_receiveByteSPI()) & 0xF3)
    {           
      cSize = resp & 0x03;         
      cSize <<= 8;           
      cSize |= sd_receiveByteSPI();
      cSize <<= 2;        
      cSize |= sd_receiveByteSPI() >> 6;
      
      cSizeMult  = (sd_receiveByteSPI() & 0x03) << 1;
      cSizeMult |=  sd_receiveByteSPI() >> 7;
      
      flag = 0;
    }
    if (timeout++ >= 0xFF) 
    {
      CS_SD_HIGH; 
      return 1; 
    }
  }

  CS_SD_HIGH;

  // Calculate memory capacity
  
  // blockLen = 2^READ_BL_LEN
  blockLen = 1;
  for (uint8_t i = 0; i < readBlckLen; i++) 
    blockLen = blockLen * 2;

  // mult = 2^(cSizeMult + 2)
  mult = 1;
  for (uint8_t i = 0; i < cSizeMult + 2; i++) 
    mult = mult * 2;

  // Memory capacity in bytes
  return ((cSize + 1) * mult * blockLen);
}

uint32_t sd_getMemoryCapacitySDHC (void)
{
  uint8_t  r1 = 0;
  uint8_t  resp;
  uint8_t  timeout = 0;
  uint64_t cSize = 0;
  uint8_t  flag;


  // SEND_CSD (CMD9)
  CS_SD_LOW;
  sd_sendCommand (SEND_CSD,0);
  r1 = sd_getR1();
  if (r1 > 0)
  {
    CS_SD_HIGH;
    return 1;
  }
  

  // Only C_SIZE is needed to calculate the memory capacity of a SDHC/SDXC
  // type cards. The CSD fields prior to the C_SIZE are used here to verfiy
  // CSD is accurately being read-in, and the correct memory capacity function
  // has been called.

  // CSD_STRUCTURE - Must be 1 for SDHC types.
  do
  { 
    if (sd_receiveByteSPI() >> 6 == 1) 
      break;
    if (timeout++ >= 0xFF)
    { 
      CS_SD_HIGH; 
      return 1; 
    }
  }
  while(1);

  // TAAC - Must be 0X0E (1ms)
  timeout = 0;
  do
  { 
    if (sd_receiveByteSPI() == 0x0E) 
      break;
    if (timeout++ >= 0xFF) 
    { 
      CS_SD_HIGH; 
      return 1; 
    }
  }
  while(1);

  // NSAC - Must be 0X00
  timeout = 0;
  do
  { 
    if (sd_receiveByteSPI() == 0x00) 
      break;
    if (timeout++ >= 0xFF) 
    {
      CS_SD_HIGH; 
      return 1; 
    }
  }
  while(1);

  // TRAN_SPEED - Must be 0x32 for current implementation
  timeout = 0;
  do
  { 
    if (sd_receiveByteSPI() == 0x32)
      break;
    if (timeout++ >= 0xFF) 
    { 
      CS_SD_HIGH; 
      return 1; 
    }
  }
  while(1);

  // Next 2 bytes contain the 12-bit CCC and 4-bit READ_BL_LEN. CCC is of the
  // form _1_1101101_1 for SDHC/SDXC. READ_BL_LEN must be 9 for SDHC/SDXC.
  flag = 1;
  timeout = 0;
  while (flag == 1)
  {
    //CCC[11:4]
    if (sd_receiveByteSPI() == 0x5B) 
    {
      //CCC[3:0] and READ_BL_LEN
      if (sd_receiveByteSPI() != 0x59) 
      {
        CS_SD_HIGH; 
        return 1;
      }
      flag = 0;
    }
    if (timeout++ >= 0xFF)
    { 
      CS_SD_HIGH;
      return 1;
    }
  }

  //This section gets the remaining bits leading up to C_SIZE.
  flag = 1;
  timeout = 0;
  while(flag == 1)
  {
    // READ_BLK_PARTIAL[7] = 0, WRITE_BLK_MISALIGN[6] = 0,
    // READ_BLK_MISALIGN[5] = 0, DSR_IMP[4] = X, RESERVERED[3:0] = 0;
    resp = sd_receiveByteSPI();
    if (resp == 0 || resp == 16) 
    {
        
        cSize = sd_receiveByteSPI() & 0x3F;       // Only [5:0] is C_SIZE          
        cSize <<= 8;           
        cSize |= sd_receiveByteSPI();
        cSize <<= 8;        
        cSize |= sd_receiveByteSPI();
        flag = 0;
    }
    if (timeout++ >= 0xFF) 
    { 
      CS_SD_HIGH; 
      return 1; 
    }
  }
  CS_SD_HIGH;

  // memory capacity in bytes
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

void sd_findNonZeroDataBlockNums (uint32_t startBlckAddr, uint32_t endBlckAddr)
{
  uint8_t  blkArr[512];                          
  uint16_t tab = 0;                              // for print format
  uint32_t address = 0;
  
  uint32_t blckNum = startBlckAddr;
  for (; blckNum <= endBlckAddr; blckNum++)
  {
    address = blckNum;
    sd_readSingleBlock (address, blkArr);       
    
    for (uint16_t i = 0; i < BLOCK_LEN; i++)
    {
      if (blkArr[i]!=0)
      {
        if (tab%5==0) 
          print_str("\n\r");
        print_str ("\t\t"); 
        print_dec (blckNum);
        tab++;
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
 *               using sd_printSingleBlock() function from SD_SPI_RWE.
 * 
 * Arguments   : startBlckAddr     Address of the first block to be printed.
 *    
 *               numOfBlcks        The number of blocks to be printed to the 
 *                                 screen starting at startBlckAddr.
 * 
 * Returns     : Read Block Error (upper byte) and R1 Response (lower byte).
 * ----------------------------------------------------------------------------
 */

uint16_t sd_printMultipleBlocks (uint32_t startBlckAddr, uint32_t numOfBlcks)
{
  uint8_t  blckArr[512];
  uint16_t timeout = 0;
  uint8_t  r1;

  CS_SD_LOW;
  sd_sendCommand (READ_MULTIPLE_BLOCK, startBlckAddr);     // CMD18
  r1 = sd_getR1();
  if (r1 > 0)
  {
    CS_SD_HIGH;
    return (R1_ERROR | r1);
  }

  if (r1 == 0)
  {
    for (uint8_t i = 0; i < numOfBlcks; i++)
    {
      print_str ("\n\r Block ");
      print_dec (startBlckAddr + i);
      timeout = 0;

      // wait for start block token.
      while (sd_receiveByteSPI() != 0xFE) 
      {
        if (timeout++ > 0xFFF) 
          return (START_TOKEN_TIMEOUT | r1);
      }

      // Load array with data from SD card block.
      for (uint16_t k = 0; k < BLOCK_LEN; k++) 
        blckArr[k] = sd_receiveByteSPI();
      
      // get CRC
      for (uint8_t k = 0; k < 2; k++) 
        sd_receiveByteSPI(); 

      // print the block to the screen.
      sd_printSingleBlock (blckArr);
    }
    
    sd_sendCommand (STOP_TRANSMISSION, 0);
    
    // get R1b response. Don't care.
    sd_receiveByteSPI(); 
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

uint16_t sd_writeMultipleBlocks (uint32_t startBlckAddr, uint32_t numOfBlcks, 
                                 uint8_t * dataArr)
{
  uint8_t  r1;
  uint16_t timeout = 0;
  uint8_t  dataRespTkn;
  uint16_t retTkn;
  uint8_t  dataTknMask = 0x1F;

  CS_SD_LOW;    
  sd_sendCommand (WRITE_MULTIPLE_BLOCK, startBlckAddr);    //CMD25
  r1 = sd_getR1();
  
  if (r1 > 0)
  {
    CS_SD_HIGH
    return (R1_ERROR | r1);
  }

  if (r1 == 0)
  {
    for (uint32_t k = 0; k < numOfBlcks; k++)
    {
      // send Start Block Token to initiate data transfer
      sd_sendByteSPI (0xFC); 

      // send data to SD card.
      for (uint16_t i = 0; i < BLOCK_LEN; i++)
        sd_sendByteSPI (dataArr[i]);

      // Send 16-bit CRC. 
      // CRC should be off (default), in which case these do not matter.
      sd_sendByteSPI (0xFF);
      sd_sendByteSPI (0xFF);
      
      // wait for valid data response token
      do
      { 
        dataRespTkn = sd_receiveByteSPI();
        if (timeout++ > 0x2FF)
        {
          CS_SD_HIGH;
          return (DATA_RESPONSE_TIMEOUT | r1);
        }  
      }
      while ((dataTknMask & dataRespTkn) != 0x05 &&        // DATA_ACCEPTED
             (dataTknMask & dataRespTkn) != 0x0B &&        // CRC_ERROR
             (dataTknMask & dataRespTkn) != 0x0D);         // WRITE_ERROR

      // Data Accepted
      if ((dataRespTkn & 0x05) == 0x05)     
      {
        timeout = 0;
        
        // Wait for SD card to finish writing data to the block.
        // Data Out (DO) line held low while card is busy writing to block.
        while (sd_receiveByteSPI() == 0)
        {
          if(timeout++ > 0x2FF) 
            return (CARD_BUSY_TIMEOUT | r1); 
        };
        retTkn = DATA_ACCEPTED_TOKEN_RECEIVED;
      }

      // CRC Error
      else if ((dataRespTkn & 0x0B) == 0x0B)
      {
        retTkn = CRC_ERROR_TOKEN_RECEIVED;
        break;
      }

      // Write Error
      else if( (dataRespTkn & 0x0D) == 0x0D ) 
      {
        retTkn = WRITE_ERROR_TOKEN_RECEIVED;
        break;
      }
    }

    timeout = 0;
    // send Stop Transmission Token
    sd_sendByteSPI (0xFD); 
    while (sd_receiveByteSPI() == 0)
    {
      if (timeout++ > 0x2FF) 
      {
        CS_SD_HIGH;
        return (CARD_BUSY_TIMEOUT | r1);
      }
    }
  }

  // Have found sometimes even after CARD_BUSY is no longer set,
  // that if another command is immediately issued, it results 
  // result in errors unless some delay is added before de-asserting
  // CS here. So that's what this for loop is doing. 
  for (uint8_t k = 0; k < 0xFE; k++)
    sd_sendByteSPI (0xFF);

  CS_SD_HIGH;

  // successful write.
  return retTkn; 
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

uint16_t sd_getNumOfWellWrittenBlocks (uint32_t * wellWrtnBlcks)
{
  uint8_t  r1;
  uint16_t timeout = 0; 

  CS_SD_LOW;
  // Send APP_CMD to signal next command is an ACMD
  sd_sendCommand (APP_CMD, 0);                             
  r1 = sd_getR1();
  if (r1 > 0) 
  {   
    CS_SD_HIGH;
    return (R1_ERROR | r1);
  }
  sd_sendCommand (SEND_NUM_WR_BLOCKS, 0);
  r1 = sd_getR1();
  if (r1 > 0)
  {
    CS_SD_HIGH;
    return (R1_ERROR | r1);
  }

  // check if Start Block Token received
  while (sd_receiveByteSPI() != 0xFE)
  {
    if(timeout++ > 0x1FF) 
    {
      CS_SD_HIGH;
      return (START_TOKEN_TIMEOUT | r1);
    }
  }
  
  // Get the number of well written blocks (32-bit)
  *wellWrtnBlcks  = sd_receiveByteSPI();
  *wellWrtnBlcks <<= 8;
  *wellWrtnBlcks |= sd_receiveByteSPI();
  *wellWrtnBlcks <<= 8;
  *wellWrtnBlcks |= sd_receiveByteSPI();
  *wellWrtnBlcks <<= 8;
  *wellWrtnBlcks |= sd_receiveByteSPI();

  // CRC bytes
  sd_receiveByteSPI();
  sd_receiveByteSPI();

  CS_SD_HIGH;

  return READ_SUCCESS;
}
