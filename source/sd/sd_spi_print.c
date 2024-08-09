/*
 * File       : SD_SPI_PRINT.C
 * Version    : 1.0
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020 - 2024
 * 
 * Implements the SD response print functions from sd_spi_print.h. 
 */

#include <stdint.h>
#include "prints.h"
#include "sd_spi_base.h"
#include "sd_spi_rwe.h"
#include "sd_spi_print.h"

/*
 * ----------------------------------------------------------------------------
 *                                                      PRINT R1 RESPONSE FLAGS
 * 
 * Description : Prints the R1 response flags returned by sd_GetR1().
 * 
 * Arguments   : r1   - The R1 response byte returned by sd_GetR1().
 * ----------------------------------------------------------------------------
 */
void sd_PrintR1(uint8_t r1)
{
  if (r1 & R1_TIMEOUT)
    print_Str(" R1_TIMEOUT,");
  if (r1 & PARAMETER_ERROR)
    print_Str(" PARAMETER_ERROR,");
  if (r1 & ADDRESS_ERROR)
    print_Str(" ADDRESS_ERROR,");
  if (r1 & ERASE_SEQUENCE_ERROR)
    print_Str(" ERASE_SEQUENCE_ERROR,");
  if (r1 & COM_CRC_ERROR)
    print_Str(" COM_CRC_ERROR,");
  if (r1 & ILLEGAL_COMMAND)
    print_Str(" ILLEGAL_COMMAND,");
  if (r1 & ERASE_RESET)
    print_Str(" ERASE_RESET,");
  if (r1 & IN_IDLE_STATE)
    print_Str(" IN_IDLE_STATE");
  if (r1 == OUT_OF_IDLE) // 0
    print_Str(" OUT_OF_IDLE");
}

/*
 * ----------------------------------------------------------------------------
 *                                          PRINT INITIALIZATION RESPONSE FLAGS
 * 
 * Description : Prints any Initialization Error Flags returned by 
 *               sd_InitModeSPI during initialization of the SD card.
 * 
 * Arguments   : initResp   - The Initialization Response returned by the
 *                            initialization routine.
 * 
 * Notes       : This function only interprets bits 8 to 16 of the response
 *               returned by sd_InitModeSPI, however, the entire response can
 *               be passed. Bits 0 to 7 represent the R1 response, and will be 
 *               ignored here. To read the R1 portion, pass it to sd_PrintR1.
 * ----------------------------------------------------------------------------
 */
void sd_PrintInitErrorResponse(uint32_t initResp)
{
  if (initResp & FAILED_GO_IDLE_STATE)
    print_Str(" FAILED_GO_IDLE_STATE,");
  if (initResp & FAILED_SEND_IF_COND)
    print_Str(" FAILED_SEND_IF_COND,");
  if (initResp & UNSUPPORTED_CARD_TYPE)
    print_Str(" UNSUPPORTED_CARD_TYPE,");
  if (initResp & FAILED_CRC_ON_OFF)
    print_Str(" FAILED_CRC_ON_OFF,");
  if (initResp & FAILED_APP_CMD)
    print_Str(" FAILED_APP_CMD,");
  if (initResp & FAILED_SD_SEND_OP_COND)
    print_Str(" FAILED_SD_SEND_OP_COND,");
  if (initResp & OUT_OF_IDLE_TIMEOUT)
    print_Str(" OUT_OF_IDLE_TIMEOUT,");
  if (initResp & FAILED_READ_OCR)
    print_Str(" FAILED_READ_OCR,");
  if (initResp & POWER_UP_NOT_COMPLETE)
    print_Str(" POWER_UP_NOT_COMPLETE,");
  if (initResp == OUT_OF_IDLE) // 0
    print_Str(" INIT_SUCCESS\n\r");
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
void sd_PrintReadError(uint8_t err)
{
  // 0xFF00 filters out the lower byte which is the R1 response
  switch (err)// & 0xFF00)
  {
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
void sd_PrintWriteError(uint8_t err)
{
  // 0xFF00 filters out the lower byte which is the R1 response
  switch(err)
  {
    case WRITE_SUCCESS:
      print_Str("\n\r WRITE_SUCCESS");
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
  switch(err)
  {
    case ERASE_SUCCESS:
      print_Str("\n\r ERASE_SUCCESS");
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



/*
 * ----------------------------------------------------------------------------
 *                                                           PRINT SINGLE BLOCK
 * 
 * Description : Print contents of a an array loaded with data from a single
 *               block on the SD card. The array's contents will be printed to 
 *               the screen in rows of 16 data bytes. Each row begins with the
 *               block offset address of the first byte in each row, a copy of 
 *               the data in HEX format, and a copy of the data in ASCII.
 * 
 * Arguments   : blckArr   - pointer to array containing contents of the data 
 *                           block to be printed to screen. 
 * 
 * Note        : Array must be of length BLOCK_LEN.
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
      print_Str("0");
    print_Hex(offset);

    // print HEX values of the block's offset row
    print_Str("   ");
    for (offset = row * radix; offset < row * radix + radix; ++offset)
    {
      // every 4 bytes print an extra space.
      if (offset % 4 == 0) 
        print_Str(" ");
      print_Str(" ");

      // if value is not two hex digits, prefix a 0. 
      if (blckArr[offset] < 0x10)
        print_Str("0");

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
      if (blckArr[offset] < ASCII_PRINT_CHAR_FIRST)    
        print_Str(" "); 
      else if (blckArr[offset] <= ASCII_PRINT_CHAR_LAST)
      { 
        // single char string to use with print_Str.
        char str[2] = {blckArr[offset], '\0'}; 
        print_Str(str);
      }
      else 
        print_Str(".");
    }
  }    
}