/*
 * File    : SD_SPI_RWE.H
 * Version : 0.0.0.1 
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : MIT
 * Copyright (c) 2020-2021
 * 
 * Implementation of SD_SPI_RWE.H
 */

#include <stdint.h>
#include <avr/io.h>
#include "usart.h"
#include "prints.h"
#include "spi.h"
#include "sd_spi_base.h"
#include "sd_spi_rwe.h"


/*
 ******************************************************************************
 *                                   FUNCTIONS   
 ******************************************************************************
 */

/*
 * For the following read, write, and erase block functions, the returned error
 * response values can be read by their corresponding print error function. For
 * example, the returned value of sd_readSingleBlock() can be read by passing
 * it to sd_printReadError(). These print functions will read the upper byte of
 * of the returned error response. If in the error response the R1_ERROR flag 
 * is set in the upper byte, then the lower byte (i.e. the R1 Response portion
 * of the error response) contains at least one flag that has been set which 
 * should then be read by passing it to sd_printR1() in SD_SPI_BASE. 
 */ 


/*
 * ----------------------------------------------------------------------------
 *                                                            READ SINGLE BLOCK
 * 
 * Description : Reads a single data block from the SD card into an array.     
 * 
 * Arguments   : blckAddr     address of the data block on the SD card that    
 *                            will be read into the array.
 * 
 *               blckArr      pointer to the array to be loaded with the       
 *                            contents of the data block at blckAddr. Must be  
 *                            length BLOCK_LEN.
 * 
 * Returns     : Read Block Error (upper byte) and R1 Response (lower byte).   
 * ----------------------------------------------------------------------------
 */

uint16_t sd_readSingleBlock (uint32_t blckAddr, uint8_t * blckArr)
{
  uint8_t timeout = 0;
  uint8_t r1 = 0;                                

  CS_SD_LOW;
  sd_sendCommand (READ_SINGLE_BLOCK, blckAddr);            //CMD17
  r1 = sd_getR1();

  if (r1 > 0)
  {
    CS_SD_HIGH;
    return (R1_ERROR | r1);
  }

  if (r1 == 0)
  {
    // wait for Start Block Token
    while (sd_receiveByteSPI() != 0xFE) 
    {
      timeout++;
      if (timeout > 0xFE) 
      { 
        CS_SD_HIGH;
        return (START_TOKEN_TIMEOUT | r1);
      }
    }

    // Load SD card block into the array.         
    for (uint16_t i = 0; i < BLOCK_LEN; i++)
      blckArr[i] = sd_receiveByteSPI();

    // Get CRC
    for (uint8_t i = 0; i < 2; i++)
      sd_receiveByteSPI();
    
    // clear any remaining data from the SPDR
    sd_receiveByteSPI();          
  }
  CS_SD_HIGH;

  return (READ_SUCCESS | r1);
}


/*
 * ----------------------------------------------------------------------------
 *                                                           PRINT SINGLE BLOCK
 * 
 * Description : Prints the contents of a single SD card data block, previously 
 *               loaded into an array by sd_readSingleBlock(), to the screen. 
 *               The block's contents will be printed to the screen in rows of
 *               16 bytes, columnized as (Addr)OFFSET | HEX | ASCII.
 * 
 * Arguments   : blckArr     pointer to an array holding the contents of the 
 *                           block to be printed to the screen. Must be of 
 *                           length BLOCK_LEN.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */

void sd_printSingleBlock (uint8_t * blckArr)
{
  uint16_t offset = 0;
  uint16_t space = 0;
  uint16_t row = 0;
  
  print_str ("\n\n\r BLOCK OFFSET\t\t\t\t   HEX\t\t\t\t\t     ASCII\n\r");
  for (; row < BLOCK_LEN/16; row++)
  {
    // print row offset address
    print_str ("\n\r   ");
    if (offset < 0x100)
    {
      print_str ("0x0"); 
      print_hex (offset);
    }
    else if (offset < 0x1000)
    {
      print_str ("0x"); 
      print_hex (offset);
    }
    
    // print HEX values of the block's row at offset
    print_str ("\t ");
    space = 0;
    for (offset = row*16; offset < row*16 + 16; offset++)
    {
      // every 4 bytes print an extra space.
      if (space % 4 == 0) 
        print_str (" ");
      print_str (" ");
      print_hex (blckArr[offset]);
      space++;
    }
    
    // print ASCII values of the block's row at offset
    print_str ("\t\t");
    offset -= 16;
    for (offset = row*16; offset < row*16 + 16; offset++)
    {
      if (blckArr[offset] < 32) 
        usart_transmit ( ' ' ); 
      else if (blckArr[offset] < 128)
        usart_transmit (blckArr[offset]);
      else 
        usart_transmit ('.');
    }
  }    
  print_str ("\n\n\r");
}
 

/*
 * ----------------------------------------------------------------------------
 *                                                           WRITE SINGLE BLOCK
 * 
 * Description : Writes the values in an array to a single SD card data block.
 * 
 * Arguments   : blckAddr     address of the data block on the SD card that 
 *                            will be written to.
 *       
 *               dataArr      pointer to an array that holds the data contents
 *                            that will be written to the block at blckAddr on
 *                            the SD card. Must be of length BLOCK_LEN.
 * 
 * Returns     : Write Block Error (upper byte) and R1 Response (lower byte).
 * ----------------------------------------------------------------------------
 */

uint16_t sd_writeSingleBlock (uint32_t blckAddr, uint8_t * dataArr)
{
  uint8_t  r1;
  uint16_t timeout = 0;
  uint8_t  dataRespTkn;
  uint8_t  dataTknMask = 0x1F;


  CS_SD_LOW;    
  sd_sendCommand (WRITE_BLOCK, blckAddr);                  // CMD24
  r1 = sd_getR1();
  if (r1 > 0)
  {
    CS_SD_HIGH;
    return (R1_ERROR | r1);
  }

  if (r1 == 0)
  {
    // send Start Block Token to initiate data transfer
    sd_sendByteSPI (0xFE); 

    // send data to write to SD card.
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
      if (timeout++ > 0xFE)
      {
        CS_SD_HIGH;
        return (DATA_RESPONSE_TIMEOUT | r1);
      }
    }
    while ((dataTknMask & dataRespTkn) != 0x05 &&          // DATA_ACCEPTED
           (dataTknMask & dataRespTkn) != 0x0B &&          // CRC_ERROR
           (dataTknMask & dataRespTkn) != 0x0D);           // WRITE_ERROR
    
    // Data Accepted
    if ((dataRespTkn & 0x05) == 0x05)                         
    {
        timeout = 0;
        
        // Wait for SD card to finish writing data to the block.
        // Data Out (DO) line held low while card is busy writing to block.
        while (sd_receiveByteSPI() == 0) 
        {
          if (timeout++ > 0x4FF) 
          {
            CS_SD_HIGH;
            return (CARD_BUSY_TIMEOUT | r1);
          }
        };
        CS_SD_HIGH;
        return (DATA_ACCEPTED_TOKEN_RECEIVED | r1);
    }

    // CRC Error
    else if ((dataRespTkn & 0x0B) == 0x0B) 
    {
      CS_SD_HIGH;
      return (CRC_ERROR_TOKEN_RECEIVED | r1);
    }

    // Write Error
    else if ((dataRespTkn & 0x0D) == 0x0D)
    {
      CS_SD_HIGH;
      return (WRITE_ERROR_TOKEN_RECEIVED | r1);
    }
  }
  return (INVALID_DATA_RESPONSE | r1) ;
}


/*
 * ----------------------------------------------------------------------------
 *                                                                 ERASE BLOCKS
 * 
 * Description : Erases the blocks between (and including) the startBlckAddr 
 *               and endBlckAddr.
 * 
 * Arguments   : startBlckAddr     address of the first block to be erased.
 * 
 *               endBlckAddr       address of the last block to be erased.
 * 
 * Returns     : Erase Block Error (upper byte) and R1 Response (lower byte).
 * ----------------------------------------------------------------------------
 */

uint16_t sd_eraseBlocks (uint32_t startBlckAddr, uint32_t endBlckAddr)
{
  uint8_t r1;
  uint16_t timeout = 0; 
  
  // set Start Address for erase block
  CS_SD_LOW;
  sd_sendCommand (ERASE_WR_BLK_START_ADDR, startBlckAddr);
  r1 = sd_getR1();
  CS_SD_HIGH;
  if (r1 > 0) 
    return (SET_ERASE_START_ADDR_ERROR | R1_ERROR | r1);
  
  // set End Address for erase block
  CS_SD_LOW;
  sd_sendCommand (ERASE_WR_BLK_END_ADDR, endBlckAddr);
  r1 = sd_getR1();
  CS_SD_HIGH;
  if (r1 > 0) 
    return (SET_ERASE_END_ADDR_ERROR | R1_ERROR | r1);

  // erase all blocks between, and including, start and end address
  CS_SD_LOW;
  sd_sendCommand(ERASE,0);
  r1 = sd_getR1 ();
  if(r1 > 0)
  {
    CS_SD_HIGH;
    return (ERASE_ERROR | R1_ERROR | r1);
  }

  // wait for card not to finish erasing blocks.
  while (sd_receiveByteSPI() == 0)
  {
    if(timeout++ > 0xFFFE) 
      return (ERASE_BUSY_TIMEOUT | r1);
  }
  CS_SD_HIGH;
  return ERASE_SUCCESSFUL;
}


/*
 * If either of the three print error functions show that the R1_ERROR flag was
 * set in the error response that was passed to it, then the error response
 * should then be passed to sd_printR1() from SD_SPI_BASE.H/C to read the 
 * R1 Error.
 */

/*
 * ----------------------------------------------------------------------------
 *                                                             PRINT READ ERROR
 * 
 * Description : Print Read Error Flag returned by a SD card read function.  
 * 
 * Arguments   : err     Read Error Response.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */

void sd_printReadError (uint16_t err)
{
  switch (err & 0xFF00)
  {
    case (R1_ERROR):
      print_str ("\n\r R1_ERROR");
      break;
    case (READ_SUCCESS):
      print_str ("\n\r READ_SUCCESS");
      break;
    case (START_TOKEN_TIMEOUT):
      print_str ("\n\r START_TOKEN_TIMEOUT");
      break;
    default:
      print_str ("\n\r UNKNOWN RESPONSE");
  }
}


/*
 * ----------------------------------------------------------------------------
 *                                                            PRINT WRITE ERROR
 * 
 * Description : Print Write Error Flag returned by a SD card read function.  
 * 
 * Arguments   : err     Write Error Response.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */

void sd_printWriteError (uint16_t err)
{
  switch(err&0xFF00)
  {
    case (DATA_ACCEPTED_TOKEN_RECEIVED):
      print_str ("\n\r DATA_ACCEPTED_TOKEN_RECEIVED");
      break;
    case (CRC_ERROR_TOKEN_RECEIVED):
      print_str ("\n\r CRC_ERROR_TOKEN_RECEIVED");
      break;
    case (WRITE_ERROR_TOKEN_RECEIVED):
      print_str ("\n\r WRITE_ERROR_TOKEN_RECEIVED");
      break;
    case (INVALID_DATA_RESPONSE):
      print_str ("\n\r INVALID_DATA_RESPONSE"); // Successful data write
      break;
    case (DATA_RESPONSE_TIMEOUT):
      print_str ("\n\r DATA_RESPONSE_TIMEOUT");
      break;
    case (CARD_BUSY_TIMEOUT):
      print_str ("\n\r CARD_BUSY_TIMEOUT");
      break;
    case (R1_ERROR):
      print_str ("\n\r R1_ERROR"); // Successful data write
      break;
    default:
      print_str ("\n\r UNKNOWN RESPONSE");
  }
}


/*
 * ----------------------------------------------------------------------------
 *                                                            PRINT ERASE ERROR
 * 
 * Description : Print Erase Error Flag returned by a SD card read function.  
 * 
 * Arguments   : err     Erase Error Response.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */

void sd_printEraseError (uint16_t err)
{
  switch(err&0xFF00)
  {
    case (ERASE_SUCCESSFUL):
      print_str ("\n\r ERASE_SUCCESSFUL");
      break;
    case (SET_ERASE_START_ADDR_ERROR):
      print_str ("\n\r SET_ERASE_START_ADDR_ERROR");
      break;
    case (SET_ERASE_END_ADDR_ERROR):
      print_str ("\n\r SET_ERASE_END_ADDR_ERROR");
      break;
    case (ERASE_ERROR):
      print_str ("\n\r ERROR_ERASE");
      break;
    case (ERASE_BUSY_TIMEOUT):
      print_str ("\n\r ERASE_BUSY_TIMEOUT");
      break;
    default:
      print_str ("\n\r UNKNOWN RESPONSE");
  }
}
