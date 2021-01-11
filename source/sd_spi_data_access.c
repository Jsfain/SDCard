/*
*******************************************************************************
*                                  AVR-SD CARD MODULE
*
* File    : SD_SPI_DATA_ACCESS.C
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
* License : MIT
* Copyright (c) 2020
* 
*
* DESCRIPTION:
* Implements some specialized raw data access functions against an SD card in 
* SPI mode from an AVR microcontroller, e.g. single- and multi-block read, 
* write, and erase as well as a few error printing functions. This requires
* SD_SPI_BASE.C/H to function.
*******************************************************************************
*/


#include <stdint.h>
#include <avr/io.h>
#include "usart.h"
#include "prints.h"
#include "spi.h"
#include "sd_spi_base.h"
#include "sd_spi_data_access.h"





/*
*******************************************************************************
*******************************************************************************
 *                     
 *                               FUNCTIONS   
 *  
*******************************************************************************
*******************************************************************************
*/

/*
-------------------------------------------------------------------------------
|                          READ A SINGLE DATA BLOCK
|                                        
| Description : Reads the single data block from the SD Card at blckAddr into 
|               the array pointed at by *blckArr.
|
| Arguments   : blckAddr    - address of the block on the SD card block whose 
|                             contents will be printed to the screen.
|             : *blckArr    - ptr to an array that will be loaded with the
|                             contents of the SD card's block at 'blckAddr'.
|                             Array should be of length BLOCK_LEN.
|
| Return      : Error response. The upper byte holds the Read Block Error Flag.
|               The lower byte holds the R1 response. Pass to 
|               sd_printReadError(). If R1_ERROR flag is set, then the R1 
|               response portion has an error. This should be read by 
|               sd_printR1().
|
| Notes       : Addressing is determined by the card type. SDHC is Block 
|               addressable, SDSC is byte addressable. This should be known 
|               before passing the address.
-------------------------------------------------------------------------------
*/

uint16_t 
sd_readSingleBlock (uint32_t blckAddr, uint8_t * blckArr)
{
  uint8_t timeout = 0;
  uint8_t r1;

  CS_SD_LOW;
  sd_sendCommand (READ_SINGLE_BLOCK, blckAddr); //CMD17
  r1 = sd_getR1();

  if (r1 > 0)
    {
      CS_SD_HIGH;
      return (R1_ERROR | r1);
    }

  if (r1 == 0)
    {
      timeout = 0;
      uint8_t sbt = sd_receiveByteSPI();
      // wait for Start Block Token
      while (sbt != 0xFE) 
        {
          sbt = sd_receiveByteSPI();
          timeout++;
          if(timeout > 0xFE) 
            { 
              CS_SD_HIGH;
              return ( START_TOKEN_TIMEOUT | r1 );
            }
        }

      // Start Block Token has been received         
      for(uint16_t i = 0; i < BLOCK_LEN; i++)
        blckArr[i] = sd_receiveByteSPI();
      for(uint8_t i = 0; i < 2; i++)
        sd_receiveByteSPI(); // CRC
          
      sd_receiveByteSPI(); // clear any remaining byte from SPDR
    }
  CS_SD_HIGH;
  return (READ_SUCCESS | r1);
}



/*
-------------------------------------------------------------------------------
|                                PRINT SINGLE BLOCK
|                                        
| Description : Prints the contents of a single block stored in an array to a
|               screen. The block's contents will be printed to the screen in
|               rows of 16 bytes, columnized as (Addr)OFFSET | HEX | ASCII.
|
| Argument    : *blckArr    - ptr to an array holding the contents of the block
|                             to be printed. Array should be length BLOCK_LEN.
|                             Array should be loaded by sd_readSinbleBlock().
-------------------------------------------------------------------------------
*/

void 
sd_printSingleBlock (uint8_t * blckArr)
{
  print_str ("\n\n\r BLOCK OFFSET\t\t\t\t   HEX\t\t\t\t\t     ASCII\n\r");
  uint16_t offset = 0;
  uint16_t space = 0;
  for (uint16_t row = 0; row < (BLOCK_LEN / 16); row++)
    {
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
      
      print_str ("\t ");
      space = 0;
      for (offset = row * 16; offset < (row * 16) + 16; offset++)
        {
          //every 4 bytes print an extra space.
          if (space % 4 == 0) 
            print_str (" ");
          print_str (" ");
          print_hex (blckArr[offset]);
          space++;
        }
      
      print_str ("\t\t");
      offset -= 16;
      for (offset = row * 16; offset < (row*16) + 16; offset++)
        {
            if (blckArr[offset] < 32) 
              usart_transmit ( ' ' ); 
            else if (blckArr[offset] < 128)
              usart_transmit (blckArr[offset]);
            else usart_transmit ('.');
        }
    }    
  print_str ("\n\n\r");
}



/*
-------------------------------------------------------------------------------
|                               WRITE SINGLE BLOCK
|                                        
| Description : Writes contents of array *dataArr to the block at 'blckAddr'
|               on the SD card. The data array should be of length BLOCK_LEN.
|               Its entire contents will be written to the SD card block.
|
| Arguments   : blckAddr   - address of the block that will be written to. 
|             : *dataArr   - ptr to an array that holds the data contents
|                            that will be written to the block on the SD Card.
|
| Return      : Error response. Upper byte holds the Write Block Error Flag.
|               Lower byte holds the R1 response. Pass to sd_printReadError().
|               If R1_ERROR flag is set, then the R1 response portion has an 
|               error which should be read by sd_printR1().
-------------------------------------------------------------------------------
*/

uint16_t 
sd_writeSingleBlock (uint32_t blckAddr, uint8_t * dataArr)
{
  uint8_t  r1;
  CS_SD_LOW;    
  sd_sendCommand (WRITE_BLOCK, blckAddr); // CMD24
  r1 = sd_getR1();

  if (r1 > 0)
    {
      CS_SD_HIGH;
      return (R1_ERROR | r1);
    }

  if (r1 == 0)
    {
      // sending Start Block Token initiates data transfer
      sd_sendByteSPI (0xFE); 

      // send data to write to SD card.
      for(uint16_t i = 0; i < BLOCK_LEN; i++) 
        sd_sendByteSPI (dataArr[i]);

      // Send 16-bit CRC. CRC is off by default so these do not matter.
      sd_sendByteSPI (0xFF);
      sd_sendByteSPI (0xFF);
      
      uint8_t  dataResponseToken;
      uint8_t  dataTokenMask = 0x1F;
      uint16_t timeout = 0;

      // wait for valid data response token
      do
        { 
            
          dataResponseToken = sd_receiveByteSPI();
          if(timeout++ > 0xFE)
            {
              CS_SD_HIGH;
              return (DATA_RESPONSE_TIMEOUT | r1);
            }  
              
        }
      while (((dataTokenMask & dataResponseToken) != 0x05) && // DATA_ACCEPTED
             ((dataTokenMask & dataResponseToken) != 0x0B) && // CRC_ERROR
             ((dataTokenMask & dataResponseToken) != 0x0D));  // WRITE_ERROR
      
      if ((dataResponseToken & 0x05) == 5) // DATA_ACCEPTED
      {
          timeout = 0;
          
          // Wait for SD card to finish writing data to the block.
          // Data Out (DO) line held low while card is busy writing to block.
          while (sd_receiveByteSPI() == 0) 
            {
              if (timeout++ > 0x2FF) 
                {
                  CS_SD_HIGH;
                  return (CARD_BUSY_TIMEOUT | r1);
                }
            };
          CS_SD_HIGH;
          return (DATA_ACCEPTED_TOKEN_RECEIVED | r1);
      }

      else if ((dataResponseToken & 0x0B) == 0x0B) // CRC Error
        {
          CS_SD_HIGH;
          return (CRC_ERROR_TOKEN_RECEIVED | r1);
        }

      else if ((dataResponseToken & 0x0D) == 0x0D) // Write Error
        {
          CS_SD_HIGH;
          return (WRITE_ERROR_TOKEN_RECEIVED | r1);
        }
    }
  return (INVALID_DATA_RESPONSE | r1) ;
}



/*
-------------------------------------------------------------------------------
|                               ERASE BLOCKS
|                                        
| Description : This function will erase the blocks between (and including) 
|               startBlckAddr and endBlckAddr.
|
| Arguments   : startBlckAddr - Address of the first block that will be erased.
 *            : endBlckAddr   - Address of the last block that will be erased.
|
| Return      : Error response. Upper byte holds the Erase Error Flag. Lower
|               byte holds the R1 response. Pass to sd_printEraseError().
|               If R1_ERROR flag is set, then the R1 response portion has an 
|               error which should be read by sd_printR1().
-------------------------------------------------------------------------------
*/

uint16_t 
sd_eraseBlocks (uint32_t startBlckAddr, uint32_t endBlckAddr)
{
    uint8_t r1 = 0;
    
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

    // erase all blocks in range of start and end address
    CS_SD_LOW;
    sd_sendCommand(ERASE,0);
    r1 = sd_getR1 ();
    if(r1 > 0)
      {
        CS_SD_HIGH;
        return (ERASE_ERROR | R1_ERROR | r1);
      }

    uint16_t timeout = 0; 

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
-------------------------------------------------------------------------------
|                           PRINT MULTIPLE BLOCKS
|                                        
| Description : Prints multiple blocks by calling the READ_MULTIPLE_BLOCKS SD
|               card command. The blocks that are read in will be printed by 
|               passing to sd_printSingleBlock().
|
| Arguments   : startBlckAddr - Address of the first block to be printed.
|             : numOfBlcks    - Number of blocks to be printed to the screen
|                               starting at startBlckAddr. 
|
| Return      : Error response. Upper byte holds a Read Block Error Flag. Lower
|               byte holds the R1 response. Pass to sd_printReadError(). If the
|               R1_ERROR flag is set, then the R1 response portion has an error 
|               which should then be read by sd_printR1().
-------------------------------------------------------------------------------
*/

uint16_t 
sd_printMultipleBlocks (uint32_t startBlckAddr, uint32_t numOfBlcks)
{
  uint8_t  blckArr[512];
  uint16_t timeout = 0;
  uint8_t  r1;

  CS_SD_LOW;
  sd_sendCommand (READ_MULTIPLE_BLOCK, startBlckAddr); // CMD18
  r1 = sd_getR1();
  if (r1 > 0)
    {
      CS_SD_HIGH
      return (R1_ERROR | r1);
    }

  if(r1 == 0)
    {
      for (uint8_t i = 0; i < numOfBlcks; i++)
        {
          print_str("\n\r Block ");print_dec(startBlckAddr + i);
          timeout = 0;
          while (sd_receiveByteSPI() != 0xFE) // wait for start block token.
            {
              timeout++;
              if (timeout > 0x511) 
                return (START_TOKEN_TIMEOUT | r1);
            }

          for (uint16_t k = 0; k < BLOCK_LEN; k++) 
            blckArr[k] = sd_receiveByteSPI();

          for (uint8_t k = 0; k < 2; k++) 
            sd_receiveByteSPI(); // CRC

          sd_printSingleBlock (blckArr);
        }
        
      sd_sendCommand (STOP_TRANSMISSION, 0);
      sd_receiveByteSPI(); // R1b response. Don't care.
    }

  CS_SD_HIGH;
  return READ_SUCCESS;
}



/*
-------------------------------------------------------------------------------
|                            WRITE TO MULTIPLE BLOCKS
|                                        
| Description : Write the contents of a byte array of length BLOCK_LEN to
|               multiple blocks of the SD card. The entire array data will be
|               copied to each block.
|
| Argument    : startBlckAddr - Address of first block to be written to.
|             : numOfBlcks    - Number of blocks to be written to.
|             : *dataArr      - Ptr to array of length BLOCK_LEN that holds
|                               the data that will be written to the SD Card.
|
| Return      : Error response. Upper byte holds a Write Block Error Flag.
|               Lower byte holds the R1 response. Pass to sd_printWriteError().
|               If the R1_ERROR flag is set, the R1 response portion has an 
|               error which should then be read by sd_printR1().
-------------------------------------------------------------------------------
*/

uint16_t 
sd_writeMultipleBlocks (uint32_t startBlckAddr, uint32_t numOfBlcks, 
                        uint8_t * dataArr)
{
  uint8_t dataResponseToken;
  uint16_t returnToken;
  uint16_t timeout = 0;

  CS_SD_LOW;    
  sd_sendCommand (WRITE_MULTIPLE_BLOCK, startBlckAddr);  //CMD25
  uint8_t r1 = sd_getR1();
  
  if(r1 > 0)
    {
      CS_SD_HIGH
      return (R1_ERROR | r1);
    }

  if(r1 == 0)
    {
      uint8_t dataTokenMask = 0x1F;

      for (uint32_t k = 0; k < numOfBlcks; k++)
        {
          sd_sendByteSPI (0xFC); // Start Block Token initiates data transfer

          // send data to SD card.
          for(uint16_t i = 0; i < BLOCK_LEN; i++)
            sd_sendByteSPI (dataArr[i]);

          // Send 16-bit CRC. CRC is off by default so these do not matter.
          sd_sendByteSPI (0xFF);
          sd_sendByteSPI (0xFF);

          uint16_t timeout = 0;
          
          // wait for valid data response token
          do
            { 
              dataResponseToken = sd_receiveByteSPI();
              if(timeout++ > 0xFF)
                {
                  CS_SD_HIGH;
                  return (DATA_RESPONSE_TIMEOUT | r1);
                }  
            }
          while (((dataTokenMask & dataResponseToken) != 0x05) &&
                 ((dataTokenMask & dataResponseToken) != 0x0B) && 
                 ((dataTokenMask & dataResponseToken) != 0x0D)); 

          if ((dataResponseToken & 0x05) == 5) // DATA ACCEPTED
            {
              timeout = 0;
              
              // Wait for SD card to finish writing data to the block.
              // Data out line held low while card is busy writing to block.
              while(sd_receiveByteSPI() == 0)
                {
                  if(timeout++ > 0x2FF) 
                    return (CARD_BUSY_TIMEOUT | r1); 
                };
              returnToken = DATA_ACCEPTED_TOKEN_RECEIVED;
            }

          else if( (dataResponseToken & 0x0B) == 0x0B ) // CRC Error
            {
              returnToken = CRC_ERROR_TOKEN_RECEIVED;
              break;
            }

          else if( (dataResponseToken & 0x0D) == 0x0D ) // Write Error
            {
              returnToken = WRITE_ERROR_TOKEN_RECEIVED;
              break;
            }
        }

      timeout = 0;
      sd_sendByteSPI(0xFD); // Stop Transmission Token
      while (sd_receiveByteSPI() == 0)
        {
          if(timeout++ > 511) 
            {
              CS_SD_HIGH;
              return (CARD_BUSY_TIMEOUT | r1);
            }
        }
    }
  CS_SD_HIGH;
  return returnToken; // successful write returns 0
}



/*
-------------------------------------------------------------------------------
|                      GET THE NUMBER OF WELL-WRITTEN BLOCKS
|                                        
| Description : This function sends the SD card command SEND_NUM_WR_BLOCKS. 
|               Call this after a failure on a WRITE_MULTIPLE_BLOCK command and
|               the Write Error Token is returned by the SD Card. This will 
|               provide the number of blocks that were successfully written
|               to before the error occurred.
|
| Argument    : *wellWrtnBlcks   - integer ptr to a value that will be updated 
|                                  by this function, and will specify the 
|                                  number of blocks successfully written to by
|                                  before the write error.
|
| Return      : Error response. Upper byte holds a Read Block Error Flag. The
|               lower byte holds the R1 response. Pass to sd_printWriteError().
|               If the R1_ERROR flag is set, the R1 response portion has an 
|               error which should then be read by sd_printR1().
-------------------------------------------------------------------------------
*/

uint16_t 
sd_getNumOfWellWrittenBlocks (uint32_t * wellWrtnBlcks)
{
  uint8_t r1;
  uint16_t timeout = 0; 

  CS_SD_LOW;
  sd_sendCommand (APP_CMD, 0); // next command is ACM
  r1 = sd_getR1();
  if (r1 > 0) 
    {   
      CS_SD_HIGH;
      return (R1_ERROR | r1);
    }
  sd_sendCommand (SEND_NUM_WR_BLOCKS, 0);
  r1 = sd_getR1();
  if(r1 > 0)
    {
      CS_SD_HIGH;
      return (R1_ERROR | r1);
    }
  while (sd_receiveByteSPI() != 0xFE) // start block token
  {
    if(timeout++ > 0x511) 
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



/*
-------------------------------------------------------------------------------
|                                PRINT READ ERROR
|                                        
| Description : Print Read Error Flag returned by a SD card read function.
|
| Argument    : err  - byte holding a Read Error Response.
-------------------------------------------------------------------------------
*/

void 
sd_printReadError (uint16_t err)
{
  switch (err & 0xFF00)
    {
      case (R1_ERROR):
        print_str ("\n\r R1 ERROR");
        break;
      case (READ_SUCCESS):
        print_str ("\n\r READ SUCCESS");
        break;
      case (START_TOKEN_TIMEOUT):
        print_str ("\n\r START TOKEN TIMEOUT");
        break;
      default:
        print_str ("\n\r UNKNOWN RESPONSE");
    }
}



/*
-------------------------------------------------------------------------------
|                                PRINT WRITE ERROR
|                                        
| Description : Print Write Error Flag returned by a SD card write function.
|
| Argument    : err  - byte holding a Read Error Response.
-------------------------------------------------------------------------------
*/

void 
sd_printWriteError (uint16_t err)
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
-------------------------------------------------------------------------------
|                                PRINT ERASE ERROR
|                                        
| Description : Print Erase Error Flag returned by an SD card erase function.
|
| Argument    : err  - byte holding a Read Error Response.
-------------------------------------------------------------------------------
*/
 
void 
sd_printEraseError (uint16_t err)
{
  switch(err&0xFF00)
    {
      case (ERASE_SUCCESSFUL):
        print_str ("\n\r ERASE SUCCESSFUL");
        break;
      case (SET_ERASE_START_ADDR_ERROR):
        print_str ("\n\r SET ERASE START ADDR ERROR");
        break;
      case (SET_ERASE_END_ADDR_ERROR):
        print_str ("\n\r SET ERASE END ADDR ERROR");
        break;
      case (ERASE_ERROR):
        print_str ("\n\r ERROR ERASE");
        break;
      case (ERASE_BUSY_TIMEOUT):
        print_str ("\n\r ERASE_BUSY_TIMEOUT");
        break;
      default:
        print_str ("\n\r UNKNOWN RESPONSE");
    }
}