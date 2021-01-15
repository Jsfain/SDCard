/******************************************************************************
 * File    : SD_SPI_RWE.H
 * Version : 0.0.0.1 
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : MIT
 * Copyright (c) 2020-2021
 * 
 * Description:
 * Implements SD_SPI_RWE.H
 * ***************************************************************************/



#include <stdint.h>
#include <avr/io.h>
#include "usart.h"
#include "prints.h"
#include "spi.h"
#include "sd_spi_base.h"
#include "sd_spi_rwe.h"



/******************************************************************************
 ******************************************************************************
 *                     
 *                               FUNCTIONS   
 *  
 ******************************************************************************
 *****************************************************************************/


/*
 * For the following block read, write, and erase block, the returned error
 * response values can be read by their corresponding print error function. For
 * example, the returned value of sd_readSingleBlock() can be read by passing
 * it to sd_printReadError(). These print functions will read the upper byte of
 * of the error response. If in the error response the R1_ERROR flag is set in
 * the upper byte, then the lower byte (i.e. the R1 Response portion of the
 * error response) contains at least one flag that has been set which should 
 * then be read by passing the error response to sd_printR1() in SD_SPI_BASE. 
 */ 



/*-----------------------------------------------------------------------------
 *                                                            READ SINGLE BLOCK
 * 
 * DESCRIPTION: 
 * Reads a single data block from the SD card into an array.   
 * 
 * ARGUMENTS:
 * 1) uint32_t blckAddr - Address of the data block on the SD card that will be
 *                        read into the array.
 * 2) uint8_t  *blckArr - Ptr to an array to be loaded with the contents of the 
 *                        data block at blckAddr. Must be of length BLOCK_LEN.
 * 
 * RETURN: 
 * uint16_t - Read Block Error (upper byte) and R1 Response (lower byte).
 * ------------------------------------------------------------------------- */
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



/*-----------------------------------------------------------------------------
 *                                                           PRINT SINGLE BLOCK
 * 
 * DESCRIPTION: 
 * Prints the contents of a single SD card data block, that's previously been 
 * loaded into an array, to the screen. The block's contents will be printed to
 * the screen in rows of 16 bytes, columnized as (Addr)OFFSET | HEX | ASCII.
 * 
 * ARGUMENTS: 
 * uint8_t  *blckArr - Ptr to an array holding the contents of the block to be 
 *                     printed to the screen. Must be of length BLOCK_LEN.
 * 
 * RETURN:
 * void
 * ------------------------------------------------------------------------- */
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



/*-----------------------------------------------------------------------------
 *                                                           WRITE SINGLE BLOCK
 * 
 * DESCRIPTION: 
 * Writes the values in an array to a single SD card data block.   
 * 
 * ARGUMENTS:
 * 1) uint32_t blckAddr - Address of the data block on the SD card that will be
 *                        written to.
 * 2) uint8_t  *dataArr - Ptr to an array that holds the data contents that
 *                        will be written to the block at blckAddr on the SD 
 *                        Card. Must be of length BLOCK_LEN.
 * 
 * RETURN: 
 * uint16_t - Write Block Error (upper byte) and R1 Response (lower byte).
 * ------------------------------------------------------------------------- */
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



/*-----------------------------------------------------------------------------
 *                                                                 ERASE BLOCKS
 * 
 * DESCRIPTION: 
 * Erases the blocks between (and including) the startBlckAddr and endBlckAddr.   
 * 
 * ARGUMENTS:
 * 1) uint32_t startBlckAddr - Address of the first block that will be erased.
 * 2) uint32_t endBlckAddr   - Address of the last block that will be erased.
 * 
 * RETURN: 
 * uint16_t - Erase Block Error (upper byte) and R1 Response (lower byte).
 * ------------------------------------------------------------------------- */
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
 * If either of the three print error functions show that the R1_ERROR flag was
 * set in the error response that was passed to it, then the error response
 * should then be passed to sd_printR1() from SD_SPI_BASE.H/C to read the 
 * R1 Error.
 */



/*-----------------------------------------------------------------------------
 *                                                             PRINT READ ERROR
 * 
 * DESCRIPTION: 
 * Print Read Error Flag returned by a SD card read function.  
 * 
 * ARGUMENTS:
 * uint16_t err - Read Error Response.
 * 
 * RETURN: 
 * void
 * ------------------------------------------------------------------------- */
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



/*-----------------------------------------------------------------------------
 *                                                            PRINT WRITE ERROR
 * 
 * DESCRIPTION: 
 * Print Write Error Flag returned by a SD card read function.  
 * 
 * ARGUMENTS:
 * uint16_t err - Write Error Response.
 * 
 * RETURN: 
 * void
 * ------------------------------------------------------------------------- */
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



/*-----------------------------------------------------------------------------
 *                                                            PRINT ERASE ERROR
 * 
 * DESCRIPTION: 
 * Print Erase Error Flag returned by a SD card read function.  
 * 
 * ARGUMENTS:
 * uint16_t err - Erase Error Response.
 * 
 * RETURN: 
 * void
 * ------------------------------------------------------------------------- */
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
