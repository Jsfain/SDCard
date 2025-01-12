/*
 * File       : SD_SPI_RWE.H
 * Version    : 1.0
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020 - 2024
 * 
 * Implementation of SD_SPI_RWE.H. 
 */

#include <stdint.h>
#include "sd_spi_base.h"
#include "sd_spi_rwe.h"

/*
 ******************************************************************************
 *                                 FUNCTIONS   
 ******************************************************************************
 */

/*
 * ----------------------------------------------------------------------------
 *                                                            READ SINGLE BLOCK
 * 
 * Description : Reads a single data block from the SD card into an array.     
 * 
 * Arguments   : blckAddr   - address of the data block on the SD card that    
 *                            will be read into the array.
 *               blckArr    - pointer to the array to be loaded with the       
 *                            contents of the data block at blckAddr. The array
 *                            must be length BLOCK_LEN.
 * 
 * Returns     : If an R1 error occurs when sending the READ command the 
 *               returned response is the R1 error and the R1_ERROR 
 *               flag is set to indicate this. If no R1 error occurs, the 
 *               function returns one of the READ BLOCK ERROR flags.   
 * ----------------------------------------------------------------------------
 */
uint16_t sd_ReadSingleBlock(uint32_t blckAddr, uint8_t blckArr[])
{
  uint8_t r1;                               // for R1 response

  // request contents of a single block on SD card at blckAddr.
  CS_ASSERT;
  sd_SendCommand(READ_SINGLE_BLOCK, blckAddr);
  r1 = sd_GetR1();
  if (r1 != OUT_OF_IDLE)
  {
    CS_DEASSERT;
    return (R1_ERROR | r1);
  }

  //
  // loop until the Start Block Token is received from the SD card,
  // indicating data from requested blckAddr is about to be sent.
  //
  for (uint8_t attempt = 0; sd_ReceiveByteFromSD() != START_BLOCK_TKN; ++attempt)
    if (attempt >= MAX_ATTEMPTS)
    {
      CS_DEASSERT;
      return (START_TOKEN_TIMEOUT);
    }

  // Load SD card block into array.         
  for (uint16_t byte = 0; byte < BLOCK_LEN; ++byte)
    blckArr[byte] = sd_ReceiveByteFromSD();

  // Get 16-bit CRC. Don't need.
  sd_ReceiveByteFromSD();
  sd_ReceiveByteFromSD();
  
  // clear any remaining data from the SPDR
  sd_ReceiveByteFromSD();          

  CS_DEASSERT;
  return (READ_SUCCESS);
}

/*
 * ----------------------------------------------------------------------------
 *                                                           WRITE SINGLE BLOCK
 * 
 * Description : Writes the values in an array to a single SD card data block.
 * 
 * Arguments   : blckAddr   - address of the data block on the SD card that 
 *                            will be written to.
 *               dataArr    - pointer to an array that holds the data contents
 *                            that will be written to the block at blckAddr on
 *                            the SD card. Must be of length BLOCK_LEN.
 * 
 * Returns     : If an R1 error occurs when sending the WRITE command, the 
 *               returned response is the R1 error and the R1_ERROR 
 *               flag is set to indicate this. If no R1 error occurs, the 
 *               function returns one of the WRITE BLOCK ERROR flags.   
 * ----------------------------------------------------------------------------
 */
uint16_t sd_WriteSingleBlock(uint32_t blckAddr, const uint8_t dataArr[])
{
  uint8_t r1;                               // for R1 response
  uint8_t dataRespTkn = 0;

  // send Write Single Block command to write data to blckAddr on SD card.
  CS_ASSERT;    
  sd_SendCommand(WRITE_BLOCK, blckAddr);
  if ((r1 = sd_GetR1()) != OUT_OF_IDLE)
  {
    CS_DEASSERT;
    return (R1_ERROR | r1);
  }

  // send Start Block Token (0xFE) to initiate data transfer
  sd_SendByteToSD(START_BLOCK_TKN); 

  // send data to write to SD card.
  for (uint16_t pos = 0; pos < BLOCK_LEN; ++pos) 
    sd_SendByteToSD(dataArr[pos]);

  // Send 16-bit CRC. CRC should be off (default), so these do not matter.
  sd_SendByteToSD(DMY_TKN);
  sd_SendByteToSD(DMY_TKN);
  
  //
  // loop until valid data response token received or function exits on 
  // max attempts reached.
  //
  for (uint8_t attempts = 0; 
          dataRespTkn != DATA_ACCEPTED_TKN
       && dataRespTkn != CRC_ERROR_TKN 
       && dataRespTkn != WRITE_ERROR_TKN;)
  {
    dataRespTkn = sd_ReceiveByteFromSD() & DATA_RESPONSE_TKN_MASK;
    if (++attempts > MAX_ATTEMPTS)
    {
      CS_DEASSERT;
      return (DATA_RESPONSE_TIMEOUT);
    }
  }
  
  //
  // if SD card signals the data was accepted by returning the Data Accepted
  // Token then the card will enter 'busy' state while it writes the data to 
  // the block. While busy, the card will hold the DO line at 0. If the max
  // attempt limit is reached then the function will return CARD_BUSY_TIMEOUT.
  //
  if (dataRespTkn == DATA_ACCEPTED_TKN)
  { 
    for (uint16_t attempts = 0; sd_ReceiveByteFromSD() == 0; ++attempts)
      if (attempts > 4 * MAX_ATTEMPTS)      // increased attempts limit
      {
        CS_DEASSERT;
        return (CARD_BUSY_TIMEOUT);
      }

    CS_DEASSERT;
    return (WRITE_SUCCESS);
  }
  else if (dataRespTkn == CRC_ERROR_TKN) 
  {
    CS_DEASSERT;
    return (CRC_ERROR_TKN_RECEIVED);
  }
  else if (dataRespTkn == WRITE_ERROR_TKN)
  {
  CS_DEASSERT;
  return (WRITE_ERROR_TKN_RECEIVED);
  }

  return (INVALID_DATA_RESPONSE) ;
}

/*
 * ----------------------------------------------------------------------------
 *                                                                 ERASE BLOCKS
 * 
 * Description : Erases the blocks between (and including) startBlckAddr and
 *               endBlckAddr.
 * 
 * Arguments   : startBlckAddr   - address of the first block to be erased.
 *               endBlckAddr     - address of the last block to be erased.
 * 
 * Returns     : Erase Block Error is returned in the upper byte and if an R1
 *               response other OUT_OF_IDLE is returned by the SD card when
 *               issuing one of the in this function, then the R1_ERROR flag is
 *               also set and the lower byte will contain this R1 response.
 * ----------------------------------------------------------------------------
 */
uint16_t sd_EraseBlocks(uint32_t startBlckAddr, uint32_t endBlckAddr)
{
  uint8_t r1;                               // for R1 responses
  
  // set Start Address for erase block
  CS_ASSERT;
  sd_SendCommand(ERASE_WR_BLK_START_ADDR, startBlckAddr);
  r1 = sd_GetR1();
  CS_DEASSERT;
  if (r1 != OUT_OF_IDLE) 
    return (SET_ERASE_START_ADDR_ERROR | R1_ERROR | r1);
  
  // set End Address for erase block
  CS_ASSERT;
  sd_SendCommand(ERASE_WR_BLK_END_ADDR, endBlckAddr);
  r1 = sd_GetR1();
  CS_DEASSERT;
  if (r1 != OUT_OF_IDLE) 
    return (SET_ERASE_END_ADDR_ERROR | R1_ERROR | r1);

  // erase all blocks between, and including, start and end address
  CS_ASSERT;
  sd_SendCommand(ERASE, 0);                 // arg = 0 for this command      
  r1 = sd_GetR1 ();
  if (r1 != OUT_OF_IDLE)
  {
    CS_DEASSERT;
    return (ERASE_ERROR | R1_ERROR | r1);
  }

  // wait for erase to finish. Busy (0) signal returned until erase completes.
  for (uint16_t attempts = 0; sd_ReceiveByteFromSD() == 0; ++attempts)
    if(attempts++ > 4 * MAX_ATTEMPTS) 
      return (ERASE_BUSY_TIMEOUT);

  CS_DEASSERT;
  return ERASE_SUCCESS;
}