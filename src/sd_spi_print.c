/*
 * File       : SD_SPI_PRINT.C
 * Version    : 0.0
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
void sd_PrintR1(uint8_t r1, void (*outs)(uint8_t))
{
  if (r1 & R1_TIMEOUT)
    print_Str(" R1_TIMEOUT,", outs);
  if (r1 & PARAMETER_ERROR)
    print_Str(" PARAMETER_ERROR,", outs);
  if (r1 & ADDRESS_ERROR)
    print_Str(" ADDRESS_ERROR,", outs);
  if (r1 & ERASE_SEQUENCE_ERROR)
    print_Str(" ERASE_SEQUENCE_ERROR,", outs);
  if (r1 & COM_CRC_ERROR)
    print_Str(" COM_CRC_ERROR,", outs);
  if (r1 & ILLEGAL_COMMAND)
    print_Str(" ILLEGAL_COMMAND,", outs);
  if (r1 & ERASE_RESET)
    print_Str(" ERASE_RESET,", outs);
  if (r1 & IN_IDLE_STATE)
    print_Str(" IN_IDLE_STATE", outs);
  if (r1 == OUT_OF_IDLE) // 0
    print_Str(" OUT_OF_IDLE", outs);
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
void sd_PrintInitErrorResponse(uint32_t initResp, void (*outs)(uint8_t))
{
  if (initResp & FAILED_GO_IDLE_STATE)
    print_Str(" FAILED_GO_IDLE_STATE,", outs);
  if (initResp & FAILED_SEND_IF_COND)
    print_Str(" FAILED_SEND_IF_COND,", outs);
  if (initResp & UNSUPPORTED_CARD_TYPE)
    print_Str(" UNSUPPORTED_CARD_TYPE,", outs);
  if (initResp & FAILED_CRC_ON_OFF)
    print_Str(" FAILED_CRC_ON_OFF,", outs);
  if (initResp & FAILED_APP_CMD)
    print_Str(" FAILED_APP_CMD,", outs);
  if (initResp & FAILED_SD_SEND_OP_COND)
    print_Str(" FAILED_SD_SEND_OP_COND,", outs);
  if (initResp & OUT_OF_IDLE_TIMEOUT)
    print_Str(" OUT_OF_IDLE_TIMEOUT,", outs);
  if (initResp & FAILED_READ_OCR)
    print_Str(" FAILED_READ_OCR,", outs);
  if (initResp & POWER_UP_NOT_COMPLETE)
    print_Str(" POWER_UP_NOT_COMPLETE,", outs);
  if (initResp == OUT_OF_IDLE) // 0
    print_Str(" INIT_SUCCESS\n\r", outs);
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
void sd_PrintReadError(uint8_t err, void (*outs)(uint8_t))
{
  // 0xFF00 filters out the lower byte which is the R1 response
  switch (err)// & 0xFF00)
  {
    case READ_SUCCESS:
      print_Str("\n\r READ_SUCCESS", outs);
      break;
    case START_TOKEN_TIMEOUT:
      print_Str("\n\r START_TOKEN_TIMEOUT", outs);
      break;
    default:
      print_Str("\n\r UNKNOWN RESPONSE", outs);
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
void sd_PrintWriteError(uint8_t err, void (*outs)(uint8_t))
{
  // 0xFF00 filters out the lower byte which is the R1 response
  switch(err)
  {
    case WRITE_SUCCESS:
      print_Str("\n\r WRITE_SUCCESS", outs);
      break;
    case CRC_ERROR_TKN_RECEIVED:
      print_Str("\n\r CRC_ERROR_TKN_RECEIVED", outs);
      break;
    case WRITE_ERROR_TKN_RECEIVED:
      print_Str("\n\r WRITE_ERROR_TKN_RECEIVED", outs);
      break;
    case INVALID_DATA_RESPONSE:
      print_Str("\n\r INVALID_DATA_RESPONSE", outs);
      break;
    case DATA_RESPONSE_TIMEOUT:
      print_Str("\n\r DATA_RESPONSE_TIMEOUT", outs);
      break;
    case CARD_BUSY_TIMEOUT:
      print_Str("\n\r CARD_BUSY_TIMEOUT", outs);
      break;
    default:
      print_Str("\n\r UNKNOWN RESPONSE", outs);
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
void sd_PrintEraseError(uint16_t err, void (*outs)(uint8_t))
{
  // 0xFF00 filters out the lower byte which is the R1 response
  switch(err)
  {
    case ERASE_SUCCESS:
      print_Str("\n\r ERASE_SUCCESS", outs);
      break;
    case SET_ERASE_START_ADDR_ERROR:
      print_Str("\n\r SET_ERASE_START_ADDR_ERROR", outs);
      break;
    case SET_ERASE_END_ADDR_ERROR:
      print_Str("\n\r SET_ERASE_END_ADDR_ERROR", outs);
      break;
    case ERASE_ERROR:
      print_Str("\n\r ERROR_ERASE", outs);
      break;
    case ERASE_BUSY_TIMEOUT:
      print_Str("\n\r ERASE_BUSY_TIMEOUT", outs);
      break;
    default:
      print_Str("\n\r UNKNOWN RESPONSE", outs);
  }
}



/*
 * ----------------------------------------------------------------------------
 *                                                           PRINT SINGLE BLOCK
 * 
 * Description : Print contents of an array loaded with data from a single
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
void sd_PrintSingleBlock(const uint8_t blckArr[], void (*outs)(uint8_t))
{
  const uint8_t radix = 16;                 // hex

  // print column headings with spaces added for formatting
  print_Str("\n\n\r "
            "BLOCK OFFSET                       "
            "HEX DATA                             "
            "ASCII DATA\n\r", outs);

  // print constents in the data block array
  for (uint16_t row = 0, offset = 0; row < BLOCK_LEN / radix; ++row)
  {
    // Print row address offset. Loop is used to print any needed prefixed 0's
    print_Str("\n\r     0x", outs);
    for (uint16_t os = offset + 1; os < 0x100; os *= radix)
      print_Str("0", outs);
    print_Num(offset, 16, outs);

    // print HEX values of the block's offset row
    print_Str("   ", outs);
    for (offset = row * radix; offset < row * radix + radix; ++offset)
    {
      // every 4 bytes print an extra space.
      if (offset % 4 == 0) 
        print_Str(" ", outs);
      print_Str(" ", outs);

      // if value is not two hex digits, prefix a 0. 
      if (blckArr[offset] < 0x10)
        print_Str("0", outs);

      // print value in hex.
      print_Num(blckArr[offset], 16, outs);
    }
    
    //
    // print the printable std. ASCII values in the block's offset row. If an 
    // ascii value less than the printable range is encountered, then a space
    // is printed. If an ascii values greater than the highest printable value
    // is encountered then a period ('.') is printed. 
    //
    print_Str("     ", outs);
    for (offset = row * radix; offset < row * radix + radix; ++offset)
    {
      if (blckArr[offset] < ASCII_PRINT_CHAR_FIRST)    
        print_Str(" ", outs); 
      else if (blckArr[offset] <= ASCII_PRINT_CHAR_LAST)
      { 
        // single char string to use with print_Str.
        char str[2] = {blckArr[offset], '\0'}; 
        print_Str(str, outs);
      }
      else 
        print_Str(".", outs);
    }
  }    
}