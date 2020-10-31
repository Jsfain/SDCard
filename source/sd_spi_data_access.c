/*
***********************************************************************************************************************
*                                                   AVR-SD CARD MODULE
*
* File    : SD_SPI_DATA_ACCESS.C
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
*
*
* DESCRIPTION:
* Implements some specialized raw data access functions against an SD card in SPI mode from an AVR microcontroller.
* Among the functions included here are single- and multi-block read, write, and erase as well as a few error printing
* functions. This file requires SD_SPI_BASE.C/H.
*
* FUNCTIONS:
*  (1)  uint16_t  SD_ReadSingleBlock (uint32_t blockAddress, uint8_t * block);
*  (2)  void      SD_PrintSingleBlock (uint8_t * block);
*  (3)  uint16_t  SD_WriteSingleBlock (uint32_t blockAddress, uint8_t * data);
*  (4)  uint16_t  SD_EraseBlocks (uint32_t startBlockAddress, uint32_t endBlockAddress);
*  (5)  uint16_t  SD_PrintMultipleBlocks (uint32_t startBlockAddress, uint32_t numberOfBlocks);
*  (6)  uint16_t  SD_WriteMultipleBlocks (uint32_t blockAddress, uint32_t numberOfBlocks, uint8_t * dataBuffer);
*  (7)  uint16_t  SD_GetNumberOfWellWrittenBlocks (uint32_t * wellWrittenBlocks);
*  (8)  void      SD_PrintReadError (uint16_t err);
*  (9)  void      SD_PrintWriteError (uint16_t err);
*  (10) void      SD_PrintEraseError (uint16_t err);
*
*
*                                                       MIT LICENSE
*
* Copyright (c) 2020 Joshua Fain
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
* documentation files (the "Software"), to deal in the Software without restriction, including without limitation the 
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to 
* permit ersons to whom the Software is furnished to do so, subject to the following conditions: The above copyright 
* notice and this permission notice shall be included in all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***********************************************************************************************************************
*/


#include <stdint.h>
#include <avr/io.h>
#include "../includes/usart.h"
#include "../includes/spi.h"
#include "../includes/prints.h"
#include "../includes/sd_spi_base.h"
#include "../includes/sd_spi_data_access.h"



/*
***********************************************************************************************************************
 *                                                        FUNCTIONS
***********************************************************************************************************************
*/

/*
***********************************************************************************************************************
 *                                               READ A SINGLE DATA BLOCK
 * 
 * Description : This function can be called to read the single data block from the SD Card at 'blockAddress' into the
 *               array pointed at by *block
 * 
 * Arguments   : blockAddress    - Unsigned integer specifying the addressed location of the SD card block whose 
 *                                 contents will be printed to the screen. Addressing is determined by the card type:
 *                                 SDHC is Block Addressable, SDSC is Byte Addressable.
 *             : *block          - Pointer to an array of length BLOCK_LEN (512 bytes). This function will load the
 *                                 contents of the SD card's block at 'blockAddress' into this array.
 * 
 * Return      : Error Response    The upper byte holds a Read Error Flag. The lower byte is the R1 response. Pass the
 *                                 returned value to SD_PrintReadError(err). If the R1_ERROR flag is set, then the R1
 *                                 response portion has an error, in which case this should be read by SD_PrintR1(err)
 *                                 from SD_SPI_BASE.
***********************************************************************************************************************
*/

uint16_t 
SD_ReadSingleBlock (uint32_t blockAddress, uint8_t *block)
{
  uint8_t timeout = 0;
  uint8_t r1;

  CS_LOW;
  SD_SendCommand (READ_SINGLE_BLOCK, blockAddress); //CMD17
  r1 = SD_GetR1();

  if (r1 > 0)
    {
      CS_HIGH;
      return (R1_ERROR | r1);
    }

  if (r1 == 0)
    {
      timeout = 0;
      uint8_t sbt = SD_ReceiveByteSPI();
      // wait for Start Block Token
      while (sbt != 0xFE) 
        {
          sbt = SD_ReceiveByteSPI();
          timeout++;
          if(timeout > 0xFE) 
            { 
              CS_HIGH;
              return ( START_TOKEN_TIMEOUT | r1 );
            }
        }

      // Start Block Token has been received         
      for(uint16_t i = 0; i < BLOCK_LEN; i++)
        block[i] = SD_ReceiveByteSPI();
      for(uint8_t i = 0; i < 2; i++)
        SD_ReceiveByteSPI(); // CRC
          
      SD_ReceiveByteSPI(); // clear any remaining byte from SPDR
    }
  CS_HIGH;
  return (READ_SUCCESS | r1);
}
// END SD_ReadSingleBlock()



/*
***********************************************************************************************************************
 *                                               PRINT SINGLE BLOCK CONTENTS
 * 
 * Description : This function can be called to print the contents of a single block stored in an array. The contents
 *               of the block will be printed to the screen in rows of 16 bytes, columnized as: OFFSET | HEX | ASCII.
 *               The HEX and ASCII columns are different forms of the block's contents. The OFFSET column is the byte
 *               byte of the first byte in each row from the first byte of the block.     
 * 
 * Argument    : *block          Pointer to a byte array of length BLOCK_LEN (512 bytes) that holds the data contents
 *                               of a single SD card block. This array should have previously been loaded by the
 *                               SD_ReadSingleBlock() function.
***********************************************************************************************************************
*/

void 
SD_PrintSingleBlock (uint8_t *block)
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
          print_hex (block[offset]);
          space++;
        }
      
      print_str ("\t\t");
      offset -= 16;
      for (offset = row * 16; offset < (row*16) + 16; offset++)
        {
            if (block[offset] < 32) 
              USART_Transmit ( ' ' ); 
            else if (block[offset] < 128)
              USART_Transmit (block[offset]);
            else USART_Transmit ('.');
        }
    }    
  print_str ("\n\n\r");
}
// END SD_PrintSingleBlock(uint8_t *byte)



/*
***********************************************************************************************************************
 *                                               WRITE TO A SINGLE BLOCK
 * 
 * Description : This function will write the contents in the *data array to the block at 'blockAddress' on the SD 
 *               card. The data array should be of length BLOCK_LEN (512 bytes) and its entire contents will be written
 *               to the SD card block.
 * 
 * Arguments   : blockAddress   - Unsigned integer specifying the address of the block that will be written to. 
 *             : *data          - Pointer to a byte array of length BLOCK_LEN (512 bytes) that holds the data contents
 *                                that will be written to block on the SD Card at blockAddress.
 * 
 * Return      : Error Response    The upper byte holds a Write Error Flag. The lower byte is the R1 response. Pass the
 *                                 returned value to SD_PrintWriteError(err). If the R1_ERROR flag is set, then the R1
 *                                 response portion has an error, in which case this should be read by SD_PrintR1(err)
 *                                 from SD_SPI_BASE.
***********************************************************************************************************************
*/

uint16_t 
SD_WriteSingleBlock (uint32_t blockAddress, uint8_t *data)
{
  uint8_t  r1;
  CS_LOW;    
  SD_SendCommand (WRITE_BLOCK, blockAddress); // CMD24
  r1 = SD_GetR1();

  if (r1 > 0)
    {
      CS_HIGH
      return (R1_ERROR | r1);
    }

  if (r1 == 0)
    {
      // sending Start Block Token initiates data transfer
      SD_SendByteSPI(0xFE); 

      // send data to write to SD card.
      for(uint16_t i = 0; i < BLOCK_LEN; i++) 
        SD_SendByteSPI (data[i]);

      // Send 16-bit CRC. CRC is off by default so these do not matter.
      SD_SendByteSPI (0xFF);
      SD_SendByteSPI (0xFF);
      
      uint8_t  dataResponseToken;
      uint8_t  dataTokenMask = 0x1F;
      uint16_t timeout = 0;

      // wait for valid data response token
      do
        { 
            
          dataResponseToken = SD_ReceiveByteSPI();
          if(timeout++ > 0xFE)
            {
              CS_HIGH;
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
          while (SD_ReceiveByteSPI() == 0) 
            {
              if (timeout++ > 0x1FF) 
                {
                  CS_HIGH;
                  return (CARD_BUSY_TIMEOUT | r1);
                }
            };
          CS_HIGH;
          return (DATA_ACCEPTED_TOKEN_RECEIVED | r1);
      }

      else if ((dataResponseToken & 0x0B) == 0x0B) // CRC Error
        {
          CS_HIGH;
          return (CRC_ERROR_TOKEN_RECEIVED | r1);
        }

      else if ((dataResponseToken & 0x0D) == 0x0D) // Write Error
        {
          CS_HIGH;
          return (WRITE_ERROR_TOKEN_RECEIVED | r1);
        }
    }
  return (INVALID_DATA_RESPONSE | r1) ;
}
// END SD_WriteSingleDataBlock()



/*
***********************************************************************************************************************
 *                                               ERASE BLOCKS
 * 
 * Description : This function will erase the blocks between (and including) startBlockAddress and endBlockAddress.
 * 
 * Argument    : startBlockAddress     - Address of the first block that will be erased.
 *             : endBlockAddress       - Address of the last block that will be erased.
 * 
 * Return      : Error Response   The upper byte holds an Erase Error Flag. The lower byte is the R1 response. Pass the
 *                                returned value to SD_PrintEraseError(err). If the R1_ERROR flag is set, then the R1
 *                                response portion has an error, in which case this should be read by SD_PrintR1(err)
 *                                from SD_SPI_BASE.
***********************************************************************************************************************
*/

uint16_t 
SD_EraseBlocks (uint32_t startBlockAddress, uint32_t endBlockAddress)
{
    uint8_t r1 = 0;
    
    // set Start Address for erase block
    CS_LOW;
    SD_SendCommand (ERASE_WR_BLK_START_ADDR, startBlockAddress);
    r1 = SD_GetR1();
    CS_HIGH;
    if (r1 > 0) 
      return (SET_ERASE_START_ADDR_ERROR | R1_ERROR | r1);
    
    // set End Address for erase block
    CS_LOW;
    SD_SendCommand (ERASE_WR_BLK_END_ADDR, endBlockAddress);
    r1 = SD_GetR1();
    CS_HIGH;
    if (r1 > 0) 
      return (SET_ERASE_END_ADDR_ERROR | R1_ERROR | r1);

    // erase all blocks in range of start and end address
    CS_LOW;
    SD_SendCommand(ERASE,0);
    r1 = SD_GetR1();
    if(r1 > 0)
      {
        CS_HIGH;
        return (ERASE_ERROR | R1_ERROR | r1);
      }

    uint16_t timeout = 0; 

    // wait for card not to finish erasing blocks.
    while (SD_ReceiveByteSPI() == 0)
      {
        if(timeout++ > 0xFFFE) 
          return (ERASE_BUSY_TIMEOUT | r1);
      }
    CS_HIGH;
    return ERASE_SUCCESSFUL;
}
// END SD_EraseBlocks()



/*
***********************************************************************************************************************
 *                                               PRINT MULTIPLE BLOCKS
 * 
 * Description : This function will print multiple blocks by calling the SD card command READ_MULTIPLE_BLOCK. Every
 *               block that is read in will be printed to the screen by calling the SD_PrintSingleBlock() function.
 *               This function is not really that useful except as a demonstration of the READ_MULTIPLE_BLOCK command.
 * 
 * Argument    : startBlockAddress     - Address of the first block that will be printed to the screen.
 *             : numberOfBlocks        - The total number of consecutive blocks that will be printed to the screen.
 * 
 * Return      : Error Response    The upper byte holds a Read Error Flag. The lower byte is the R1 response. Pass the
 *                                 returned value to SD_PrintReadError(err). If the R1_ERROR flag is set, then the R1
 *                                 response portion has an error, in which case this should be read by SD_PrintR1(err)
 *                                 from SD_SPI_BASE.
***********************************************************************************************************************
*/

uint16_t 
SD_PrintMultipleBlocks (uint32_t startBlockAddress, uint32_t numberOfBlocks)
{
  uint8_t  block[512];
  uint16_t timeout = 0;
  uint8_t  r1;

  CS_LOW;
  SD_SendCommand (READ_MULTIPLE_BLOCK, startBlockAddress); // CMD18
  r1 = SD_GetR1();
  if (r1 > 0)
    {
      CS_HIGH
      return (R1_ERROR | r1);
    }

  if(r1 == 0)
    {
      for (int i = 0; i < numberOfBlocks; i++)
        {
          timeout = 0;
          while (SD_ReceiveByteSPI() != 0xFE) // wait for start block token.
            {
              timeout++;
              if (timeout > 0x511) 
                return (START_TOKEN_TIMEOUT | r1);
            }

          for (uint16_t k = 0; k < BLOCK_LEN; k++) 
            block[k] = SD_ReceiveByteSPI();

          for (uint8_t k = 0; k < 2; k++) 
            SD_ReceiveByteSPI(); // CRC

          SD_PrintSingleBlock (block);
        }
        
      SD_SendCommand (STOP_TRANSMISSION, 0);
      SD_ReceiveByteSPI(); // R1b response. Don't care.
    }

  CS_HIGH;
  return READ_SUCCESS;
}
// END sd_PrintMultiplDataBlocks()



/*
***********************************************************************************************************************
 *                                               WRITE TO MULTIPLE BLOCKS
 * 
 * Description : This function will write data in a byte array of length BLOCK_LEN (512 bytes) to multiple blocks on 
 *               the SD card. The full array of data will be copied to each of the blocks. This function is not really
 *               useful except to demontrate the WRITE_MULTIPLE_BLOCK SD card command.
 * 
 * Argument    : blockAddress          - Address of the first block that will be written.
 *             : numberOfBlocks        - The total number of consecutive blocks that will be written.
 *             : *dataBuffer           - Pointer to a byte array of length BLOCK_LEN (512 bytes) that holds the data
 *                                       contents that will be written to the SD Card.
 * 
 * Return      : Error Response    The upper byte holds a Write Error Flag. The lower byte is the R1 response. Pass the
 *                                 returned value to SD_PrintWriteError(err). If the R1_ERROR flag is set, then the R1
 *                                 response portion has an error, in which case this should be read by SD_PrintR1(err)
 *                                 from SD_SPI_BASE.
***********************************************************************************************************************
*/

uint16_t 
SD_WriteMultipleBlocks (uint32_t startBlockAddress, uint32_t numberOfBlocks, uint8_t *data)
{
  uint8_t dataResponseToken;
  uint16_t returnToken;
  uint16_t timeout = 0;

  CS_LOW;    
  SD_SendCommand (WRITE_MULTIPLE_BLOCK, startBlockAddress);  //CMD25
  uint8_t r1 = SD_GetR1();
  
  if(r1 > 0)
    {
      CS_HIGH
      return (R1_ERROR | r1);
    }

  if(r1 == 0)
    {
      uint8_t dataTokenMask = 0x1F;

      for (uint32_t k = 0; k < numberOfBlocks; k++)
        {
          SD_SendByteSPI (0xFC); // Start Block Token initiates data transfer

          // send data in 'data' to SD card.
          for(uint16_t i = 0; i < BLOCK_LEN; i++)
            SD_SendByteSPI (data[i]);

          // Send 16-bit CRC. CRC is off by default so these do not matter.
          SD_SendByteSPI (0xFF);
          SD_SendByteSPI (0xFF);

          uint16_t timeout = 0;
          
          // wait for valid data response token
          do
            { 
              dataResponseToken = SD_ReceiveByteSPI();
              if(timeout++ > 0xFF)
                {
                  CS_HIGH;
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
              while(SD_ReceiveByteSPI() == 0)
                {
                  if(timeout++ > 511) 
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
      SD_SendByteSPI(0xFD); // Stop Transmission Token
      while (SD_ReceiveByteSPI() == 0)
        {
          if(timeout++ > 511) 
            {
              CS_HIGH;
              return (CARD_BUSY_TIMEOUT | r1);
            }
        }
    }
  CS_HIGH;
  return returnToken; // successful write returns 0
}
// END SD_WriteMultipleDataBlocks()



/*
***********************************************************************************************************************
 *                                        GET THE NUMBER OF WELL-WRITTEN BLOCKS
 * 
 * Description : This function can be called after a failure on a WRITE_MULTIPLE_BLOCK command and the Write Error 
 *               Token was returned by the SD Card. This function will issue the SD card command SEND_NUM_WR_BLOCKS
 *               which will return the number of blocks that were successfully written to.
 * 
 * Argument    : *wellWrittenBlocks       - Pointer to an integer that will be updated by this function to specify the 
 *                                          number of well-written blocks.
 * 
 * Return      : Error Response    The upper byte holds a Read Error Flag. The lower byte is the R1 response. Pass the
 *                                 returned value to SD_PrintReadError(err). If the R1_ERROR flag is set then the R1
 *                                 response portion has an error in which case this should be read by SD_PrintR1(err)
 *                                 from SD_SPI_BASE.
 ***********************************************************************************************************************
*/

uint16_t 
SD_GetNumberOfWellWrittenBlocks (uint32_t *wellWrittenBlocks)
{
  uint8_t r1;
  uint16_t timeout = 0; 

  CS_LOW;
  SD_SendCommand (APP_CMD, 0); // next command is ACM
  r1 = SD_GetR1();
  if (r1 > 0) 
    {   
      CS_HIGH;
      return (R1_ERROR | r1);
    }
  SD_SendCommand (SEND_NUM_WR_BLOCKS, 0);
  r1 = SD_GetR1();
  if(r1 > 0)
    {
      CS_HIGH;
      return (R1_ERROR | r1);
    }
  while (SD_ReceiveByteSPI() != 0xFE) // start block token
  {
    if(timeout++ > 0x511) 
      {
        CS_HIGH;
        return (START_TOKEN_TIMEOUT | r1);
      }
  }
  
  // Get the number of well written blocks (32-bit)
  *wellWrittenBlocks  = SD_ReceiveByteSPI();
  *wellWrittenBlocks <<= 8;
  *wellWrittenBlocks |= SD_ReceiveByteSPI();
  *wellWrittenBlocks <<= 8;
  *wellWrittenBlocks |= SD_ReceiveByteSPI();
  *wellWrittenBlocks <<= 8;
  *wellWrittenBlocks |= SD_ReceiveByteSPI();

  // CRC bytes
  SD_ReceiveByteSPI();
  SD_ReceiveByteSPI();

  CS_HIGH;

  return READ_SUCCESS;
}
// END SD_GetNumberOfWellWrittenBlocks()



/*
***********************************************************************************************************************
 *                                                 PRINT READ ERROR
 * 
 * Description : This function can be called to print the Read Error Flag returned by one of the read functions.
 * 
 * Argument    : err       Unsigned integer that should be a response returned by a read function.
***********************************************************************************************************************
*/

void 
SD_PrintReadError (uint16_t err)
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
// END SD_PrintWriteError()



/*
***********************************************************************************************************************
 *                                                PRINT WRITE ERROR
 * 
 * Description : This function can be called to print a Write Error Flag returned by one of the write functions.
 * 
 * Argument    : err       Unsigned integer that should be a response returned by a write function.
***********************************************************************************************************************
*/

void 
SD_PrintWriteError (uint16_t err)
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
// END SD_PrintWriteError()



/*
***********************************************************************************************************************
 *                                               PRINT ERASE ERROR
 * 
 * Description : This function can be called to print an Erase Error Flag returned by the erase function.
 * 
 * Argument    : err       Unsigned integer that should be a response returned by an erase function.
***********************************************************************************************************************
*/
 
void 
SD_PrintEraseError (uint16_t err)
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
// END SD_PrintWriteError()
