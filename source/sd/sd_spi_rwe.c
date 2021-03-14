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
#include "usart0.h"
#include "prints.h"
#include "spi.h"
#include "sd_spi_base.h"
#include "sd_spi_rwe.h"

/*
 ******************************************************************************
 *                                 FUNCTIONS   
 ******************************************************************************
 */

/*
 * For the following read, write, and erase block functions, the returned error
 * response values can be read by their corresponding print error function. For
 * example, the returned value of sd_ReadSingleBlock() can be read by passing
 * it to sd_PrintReadError(). These print functions will read the upper byte of
 * the returned error response. If in the error response the R1_ERROR flag is 
 * set in the upper byte, then the lower byte (i.e. the R1 Response portion of
 * the error response) contains at least one flag that has been set which 
 * should then be read by passing it to sd_PrintR1() in SD_SPI_BASE. 
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
 *                            contents of the data block at blckAddr. Must be  
 *                            length BLOCK_LEN.
 * 
 * Returns     : Read Block Error (upper byte) and R1 Response (lower byte).   
 * ----------------------------------------------------------------------------
 */
uint16_t sd_ReadSingleBlock(const uint32_t blckAddr, uint8_t blckArr[])
{
  uint8_t r1;                               // for R1 responses

  // request contents of a single data block at blckAddr on the SD card.
  CS_SD_LOW;
  sd_SendCommand(READ_SINGLE_BLOCK, blckAddr);
  r1 = sd_GetR1();
  if (r1 != OUT_OF_IDLE)
  {
    CS_SD_HIGH;
    return (R1_ERROR | r1);
  }

  //
  // loop until the 'Start Block Token' has been received from the SD card,
  // which indicates data from requested blckAddr is about to be sent.
  //
  for (uint8_t timeout = 0; sd_ReceiveByteSPI() != START_BLOCK_TKN; ++timeout)
    if (timeout >= TIMEOUT_LIMIT)
    {
      CS_SD_HIGH;
      return (START_TOKEN_TIMEOUT | r1);
    }

  // Load SD card block into the array.         
  for (uint16_t byte = 0; byte < BLOCK_LEN; ++byte)
    blckArr[byte] = sd_ReceiveByteSPI();

  // Get 16-bit CRC. Don't need.
  sd_ReceiveByteSPI();
  sd_ReceiveByteSPI();
  
  // clear any remaining data from the SPDR
  sd_ReceiveByteSPI();          

  CS_SD_HIGH;
  return (READ_SUCCESS | r1);
}

/*
 * ----------------------------------------------------------------------------
 *                                                           PRINT SINGLE BLOCK
 * 
 * Description : Prints the contents of a single SD card data block to the 
 *               screen, previously loaded into an array by sd_ReadSingleBlock. 
 *               The block's contents will be printed to the screen in rows of
 *               16 bytes, columnized as (Addr)OFFSET | HEX | ASCII.
 * 
 * Arguments   : blckArr   - pointer to an array holding the contents of the 
 *                           block to be printed to the screen. Must be of 
 *                           length BLOCK_LEN.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */
void sd_PrintSingleBlock(const uint8_t blckArr[])
{
  const uint8_t radix = 16;                 // hex

  // print column headings with spaces added for formatting
  print_Str("\n\n\r "
            "BLOCK OFFSET                       "
            "HEX DATA                             "
            "ASCII DATA\n\r");

  // print constents in the data block array
  for (uint16_t row = 0, offset = 0; row < BLOCK_LEN / radix; ++row)
  {
    // Print row address offset. Loop is used to print any needed prefixed 0's
    print_Str("\n\r     0x");
    for (uint16_t os = offset + 1; os < 0x100; os *= radix)
      usart_Transmit('0');
    print_Hex(offset);

    // print HEX values of the block's offset row
    print_Str("   ");
    for (offset = row * radix; offset < row * radix + radix; ++offset)
    {
      // every 4 bytes print an extra space.
      if (offset % 4 == 0) 
        usart_Transmit(' ');
      usart_Transmit(' ');

      // if value is not two hex digits, then first print a 0. 
      if (blckArr[offset] < 0x10)
        usart_Transmit('0');

      // print value in hex.
      print_Hex(blckArr[offset]);
    }
    
    //
    // print the printable std. ASCII values in the block's offset row. If an 
    // ascii value less than the printable range is encountered, then a space
    // is printed. If an ascii values greater than the highest printable value
    // is encountered then a period ('.') is printed. 
    //
    print_Str("     ");
    for (offset = row * radix; offset < row * radix + radix; ++offset)
    {
      if (blckArr[offset] < STD_ASCII_PRINT_RANGE_FIRST)    
        usart_Transmit(' '); 
      else if (blckArr[offset] <= STD_ASCII_PRINT_RANGE_LAST)
        usart_Transmit(blckArr[offset]);
      else 
        usart_Transmit('.');
    }
  }    
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
 * Returns     : Write Block Error (upper byte) and R1 Response (lower byte).
 * ----------------------------------------------------------------------------
 */
uint16_t sd_WriteSingleBlock(const uint32_t blckAddr, const uint8_t dataArr[])
{
  uint8_t  r1;                              // for R1 response
  uint8_t  dataRespTkn = 0;
  //uint16_t timeout = 0;

  // send the Write Single Block command to write data to blckAddr on SD card.
  CS_SD_LOW;    
  sd_SendCommand (WRITE_BLOCK, blckAddr);
  if ((r1 = sd_GetR1()) != OUT_OF_IDLE)
  {
    CS_SD_HIGH;
    return (R1_ERROR | r1);
  }

  // send Start Block Token (0xFE) to initiate data transfer
  sd_SendByteSPI(0xFE); 

  // send data to write to SD card.
  for (uint16_t pos = 0; pos < BLOCK_LEN; pos++) 
    sd_SendByteSPI (dataArr[pos]);

  // Send 16-bit CRC. CRC should be off (default), so these do not matter.
  sd_SendByteSPI(DMY_TKN);
  sd_SendByteSPI(DMY_TKN);
  
  // loop until valid data response token received or function exits on timeout
  for (uint8_t timeout = 0; 
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
  // Token then the card will enter 'busy' state while it writes the data to 
  // the block. While busy, the card will hold the DI line at 0. If the timeout
  // limit is reached then the function will return CARD_BUSY_TIMEOUT.
  //
  if (dataRespTkn == DATA_ACCEPTED_TKN)
  { 
    for (uint16_t timeout = 0; sd_ReceiveByteSPI() == 0; ++timeout)
      if (timeout > 4 * TIMEOUT_LIMIT)      // increased timeout limit
      {
        CS_SD_HIGH;
        return (CARD_BUSY_TIMEOUT | r1);
      }

    CS_SD_HIGH;
    return (DATA_WRITE_SUCCESS | r1);
  }
  else if (dataRespTkn == CRC_ERROR_TKN) 
  {
    CS_SD_HIGH;
    return (CRC_ERROR_TKN_RECEIVED | r1);
  }
  else if (dataRespTkn == WRITE_ERROR_TKN)
  {
  CS_SD_HIGH;
  return (WRITE_ERROR_TKN_RECEIVED | r1);
  }

  return (INVALID_DATA_RESPONSE | r1) ;
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
 * Returns     : Erase Block Error (upper byte) and R1 Response (lower byte).
 * ----------------------------------------------------------------------------
 */
uint16_t sd_EraseBlocks(uint32_t startBlckAddr, uint32_t endBlckAddr)
{
  uint8_t r1;                               // for R1 responses
  
  // set Start Address for erase block
  CS_SD_LOW;
  sd_SendCommand(ERASE_WR_BLK_START_ADDR, startBlckAddr);
  r1 = sd_GetR1();
  CS_SD_HIGH;
  if (r1 != OUT_OF_IDLE) 
    return (SET_ERASE_START_ADDR_ERROR | R1_ERROR | r1);
  
  // set End Address for erase block
  CS_SD_LOW;
  sd_SendCommand(ERASE_WR_BLK_END_ADDR, endBlckAddr);
  r1 = sd_GetR1();
  CS_SD_HIGH;
  if (r1 != OUT_OF_IDLE) 
    return (SET_ERASE_END_ADDR_ERROR | R1_ERROR | r1);

  // erase all blocks between, and including, start and end address
  CS_SD_LOW;
  sd_SendCommand(ERASE, 0);                 // arg = 0 for this command      
  r1 = sd_GetR1 ();
  if (r1 != OUT_OF_IDLE)
  {
    CS_SD_HIGH;
    return (ERASE_ERROR | R1_ERROR | r1);
  }

  // wait for erase to finish. Busy (0) signal returned until erase completes.
  for (uint16_t timeout = 0; sd_ReceiveByteSPI() == 0; ++timeout)
    if(timeout++ > 4 * TIMEOUT_LIMIT) 
      return (ERASE_BUSY_TIMEOUT | r1);

  CS_SD_HIGH;
  return ERASE_SUCCESSFUL;
}

/*
 * If either of the three print error functions below show that the R1_ERROR 
 * flag was set in the error response that was passed to it, then the error 
 * response should also be passed to sd_PrintR1() from SD_SPI_BASE.H/C to read 
 * the R1 Error.
 */

/*
 * ----------------------------------------------------------------------------
 *                                                             PRINT READ ERROR
 * 
 * Description : Print Read Error Flag returned by a SD card read function.  
 * 
 * Arguments   : err   - Read Error Response.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */
void sd_PrintReadError(uint16_t err)
{
  // 0xFF00 filters out the lower byte which is the R1 response
  switch (err & 0xFF00)
  {
    case R1_ERROR:
      print_Str("\n\r R1_ERROR");
      break;
    case READ_SUCCESS:
      print_Str("\n\r READ_SUCCESS");
      break;
    case START_TOKEN_TIMEOUT:
      print_Str("\n\r START_TOKEN_TIMEOUT");
      break;
    default:
      print_Str("\n\r UNKNOWN RESPONSE");
  }
}

/*
 * ----------------------------------------------------------------------------
 *                                                            PRINT WRITE ERROR
 * 
 * Description : Print Write Error Flag returned by a SD card read function.  
 * 
 * Arguments   : err   - Write Error Response.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */
void sd_PrintWriteError(uint16_t err)
{
  // 0xFF00 filters out the lower byte which is the R1 response
  switch(err & 0xFF00)
  {
    case DATA_WRITE_SUCCESS:
      print_Str("\n\r DATA_WRITE_SUCCESS");
      break;
    case CRC_ERROR_TKN_RECEIVED:
      print_Str("\n\r CRC_ERROR_TKN_RECEIVED");
      break;
    case WRITE_ERROR_TKN_RECEIVED:
      print_Str("\n\r WRITE_ERROR_TKN_RECEIVED");
      break;
    case INVALID_DATA_RESPONSE:
      print_Str("\n\r INVALID_DATA_RESPONSE");
      break;
    case DATA_RESPONSE_TIMEOUT:
      print_Str("\n\r DATA_RESPONSE_TIMEOUT");
      break;
    case CARD_BUSY_TIMEOUT:
      print_Str("\n\r CARD_BUSY_TIMEOUT");
      break;
    case R1_ERROR:
      print_Str("\n\r R1_ERROR");
      break;
    default:
      print_Str("\n\r UNKNOWN RESPONSE");
  }
}


/*
 * ----------------------------------------------------------------------------
 *                                                            PRINT ERASE ERROR
 * 
 * Description : Print Erase Error Flag returned by a SD card read function.  
 * 
 * Arguments   : err   - Erase Error Response.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */
void sd_PrintEraseError(uint16_t err)
{
  // 0xFF00 filters out the lower byte which is the R1 response
  switch(err & 0xFF00)
  {
    case ERASE_SUCCESSFUL:
      print_Str("\n\r ERASE_SUCCESSFUL");
      break;
    case SET_ERASE_START_ADDR_ERROR:
      print_Str("\n\r SET_ERASE_START_ADDR_ERROR");
      break;
    case SET_ERASE_END_ADDR_ERROR:
      print_Str("\n\r SET_ERASE_END_ADDR_ERROR");
      break;
    case ERASE_ERROR:
      print_Str("\n\r ERROR_ERASE");
      break;
    case ERASE_BUSY_TIMEOUT:
      print_Str("\n\r ERASE_BUSY_TIMEOUT");
      break;
    default:
      print_Str("\n\r UNKNOWN RESPONSE");
  }
}
