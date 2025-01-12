/*
 * File       : SD_TEST.C
 * Author     : Joshua Fain
 * Target     : ATMega1280
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020 - 2024
 *
 * Test file to demonstrate the implementation of several functions defined in
 * the SD_SPI module files. main is organized in various sections that can be 
 * enabled independently using the SD Card - Feature Testing Macros provided in
 * this file.
 * 
 * CAUTION : some of these tests have the capability of erasing and/or writing 
 * over data on an SD card.
 */

#include <avr/io.h>
#include "avr_usart.h"
#include "avr_spi.h"
#include "prints.h"
//#include "sd_spi_interface.h"
#include "sd_spi_base.h"
#include "sd_spi_rwe.h"
#include "sd_spi_misc.h"
#include "sd_spi_print.h"


#define SD_CARD_INIT_ATTEMPTS_MAX      5
#define MAX_DATA_BYTES_32_BIT          2147483648 
#define MAX_BLOCK_NUM_32_BIT           (MAX_DATA_BYTES_32_BIT / BLOCK_LEN)
#define BACKSPACE                      127  // used for keyboard backspace here

// 
// SD Card - Feature Testing Macros
//
// Used to enable the various test sections in main below. Descriptions of each
// test and its required parameter settings follow in the subsequent sections.
//
#define TEST_READ_AND_PRINT_SINGLE_BLOCK           0
#define TEST_READ_AND_PRINT_MULTIPLE_BLOCKS        0
#define TEST_ERASE_WRITE_SINGLE_BLOCK              1
#define TEST_COPY_SINGLE_BLOCK                     0
#define TEST_ERASE_WRITE_MULTIPLE_BLOCKS           0
#define TEST_INTERACTIVE_USER_SECTION              0
#define TEST_MEMORY_CAPACITY                       0
#define TEST_FIND_NONZERO_DATA_BLOCKS              0

//
// ----------------------------------------------------------------------------
//                                             TEST_READ_AND_PRINT_SINGLE_BLOCK
// 
// Demos the functions sd_ReadSingleBlock and sd_PrintSingleBlock. This test 
// operates by calling sd_ReadSingleBlock to read the contents of the block at
// the address specified by BLK_ADDR_RSB (defined here) into an array. This 
// array will then be passed to sd_PrintSingleBlock which will print the 
// block's contents to the screen. 
//
#if TEST_READ_AND_PRINT_SINGLE_BLOCK
#define BLK_ADDR_RSB         16384          // addr of blk to read in and print
#endif

// ----------------------------------------------------------------------------
//                                          TEST_READ_AND_PRINT_MULTIPLE_BLOCKS
// 
// Demos sd_PrintMultipleBlocks(). This will print the number of blocks 
// specified by NUM_OF_BLKS_PMB, beginning at START_BLK_ADDR_PMB. The function 
// calls the READ_MULTIPLE_BLOCKS SD card command.
//
#if TEST_READ_AND_PRINT_MULTIPLE_BLOCKS
#define START_BLK_ADDR_PMB   16384          // addr of first blk to print
#define NUM_OF_BLKS_PMB      5              // total num of blks to print
#endif

// ----------------------------------------------------------------------------
//                                                TEST_ERASE_WRITE_SINGLE_BLOCK
//
// Demos the functions sd_WriteSingleBlock, sd_EraseBlocks, sd_ReadSingleBlock,
// and sd_printSingleBlock. This test operates by first erasing the single 
// block at the block address specified by BLK_ADDR_WSB using sd_EraseBlocks,
// then reading in and printing this block to confirm it has been erased using
// sd_ReadSingleBlock and sd_PrintSingleBlock. Next, sd_WriteSingleBlock is 
// called to write the data specified by WRITE_STR_WSB to the block just 
// erased. Finally, it will read in and print the block again to confirm the 
// block write was successful.
//
#if TEST_ERASE_WRITE_SINGLE_BLOCK
// string/data to be written to specified block
#define WRITE_STR_WSB        "Well Hi, I See you brought a PIE!!!"
#define BLK_ADDR_WSB         20            // block that will be written to
#endif

// ----------------------------------------------------------------------------
//                                                       TEST_COPY_SINGLE_BLOCK
//
// The corresponding section in main will copy the contents in the block
// specified by SOURCE_BLK_CSB into the block specified by DEST_BLK_CSB. It
// does this by reading the contents at SOURCE_BLK_CSB into an array using 
// sd_ReadSingleBlock and then passing it to sd_WriteSingleBlock to write its
// contents to DEST_BLK_CSB.
//
#if TEST_COPY_SINGLE_BLOCK
#define SOURCE_BLK_CSB       0    // block holding data that will be copied
#define DEST_BLK_CSB         20   // block where data will be copied to
#endif

// ----------------------------------------------------------------------------
//                                             TEST_ERASE_WRITE_MULTIPLE_BLOCKS
//
// Demos sd_WriteMultipleBlocks, sd_EraseBlocks, and sd_PrintMultipleBlocks. 
// This test operates by first calling sd_EraseBlocks to erase the number of 
// blocks specified by NUM_OF_BLKS_WMB, beginning at START_BLK_ADDR_WMB. It 
// then reads-in and prints the same blocks using sd_PrintMultipleBlocks to 
// verify the blocks have been erased. Next, it writes to these blocks by 
// calling sd_WriteMultipleBlocks. Finally, it will read-in and print the same 
// blocks again to show a successful write. The data/string written to the 
// blocks is the value of WRITE_STR_WMB. This data/string will be copied to the
// beginning of each block written to. Error handling may be performed if a 
// write error occurs and the WRITE_ERROR_TKN_RECEIVED response is returned, but
// this HAS NOT been tested.
//
#if TEST_ERASE_WRITE_MULTIPLE_BLOCKS
// Data (string) to be written to the block 
#define WRITE_STR_WMB        "Would you like to play a game???"
#define START_BLK_ADDR_WMB   20             // start block for erasing
#define NUM_OF_BLKS_WMB      2              // used for writing
#endif

// ----------------------------------------------------------------------------
//                                                TEST_INTERACTIVE_USER_SECTION 
//
// This test allows users to interact with the function sd_PrintMultipleBlocks,
// and no additional parameter settings are needed for this. The test operates
// by asking a user for a starting block number, i.e. first data block they want
// to print, and the total number of blocks to print, beginning with the same 
// starting block. The sd_PrintMultipleBlocks function is then called with 
// these parameters and the blocks specified are printed. This test uses the 
// local function enterBlockNumber, to get the user-entered block number.
//
#if TEST_INTERACTIVE_USER_SECTION
// local function used to get block number from user
uint32_t enterBlockNumber();  
#endif

// ----------------------------------------------------------------------------
//                                                         TEST_MEMORY_CAPACITY
//
// The section calls the function sd_GetCardByteCapacity which will calculate
// and return the card's data capacity in bytes. No additional macro parameters
// are required for this function.
//

// ----------------------------------------------------------------------------
//                                                TEST_FIND_NONZERO_DATA_BLOCKS
//
// Demos the function sd_FindNonZeroDataBlockNums which searches for any blocks
// in the range of START_BLK_ADDR_FNZDB and END_BLK_ADDR_FNZDB (inclusive) that
// have any non-zero data in them and then prints their block address/number. 
// The function tested here was created to help locate raw data on the SD card,
// but it is not fast in printing to the screen, so it's recommended to test 
// over a small range of blocks first.
//
#if TEST_FIND_NONZERO_DATA_BLOCKS
// block number range to search for non-zero data.
#define START_BLK_ADDR_FNZDB  0 
#define END_BLK_ADDR_FNZDB    10000
#endif

int main(void)                                        
{
  // Initialize usart. Required for any printing to terminal.
  usart_Init();

  //
  // SD CARD INITILIAIZATION
  //
  // The first step to using this SD Card module is to initialize the card 
  // into SPI mode. This is done by calling sd_InitModeSPI, passing a to an 
  // instance of CTV. The members of the CTV instance will be set by the 
  // initialization routine. The value of the 'type' member is used to 
  // determine how the data blocks on the card will be addressed.
  //
  uint32_t initResp;
  CTV ctv;

  // Loop continues until SD card init succeeds or max attempts reached.
  for (uint8_t att = 0; att < SD_CARD_INIT_ATTEMPTS_MAX; ++att)
  {
    print_Str("\n\n\r >> SD Card Initialization Attempt "); 
    print_Dec(att); print_Str(":");
    initResp = sd_InitSpiMode(&ctv);        // init SD Card

    if (initResp != OUT_OF_IDLE)            // Fail to init if not OUT_OF_IDLE
    {    
      print_Str(" FAILED. Initialization Error Response: "); 
      sd_PrintInitErrorResponse(initResp);
      print_Str(" R1 Response: "); 
      sd_PrintR1(initResp);
    }
    else
    {   
      print_Str(" SUCCESS");
      break;
    }
    
  }

  if (initResp == OUT_OF_IDLE)              // initialization successful
  {      
    // ------------------------------------------------------------------------
    // BEGIN TEST                              TEST_READ_AND_PRINT_SINGLE_BLOCK
    //
    #if TEST_READ_AND_PRINT_SINGLE_BLOCK

    uint8_t  blckArrRSB[BLOCK_LEN];         // array to hold block contents
    uint16_t errRSB;                        // for returned errors

    // read single block into array. Determine if byte or block addressable.
    if (ctv.type == SDHC)
      errRSB = sd_ReadSingleBlock(BLK_ADDR_RSB, blckArrRSB);
    else
      errRSB = sd_ReadSingleBlock((uint64_t)BLK_ADDR_RSB * BLOCK_LEN, 
                                   blckArrRSB);
    
    if (errRSB == READ_SUCCESS)             // if block read was successful
      sd_PrintSingleBlock(blckArrRSB);      // then print block loaded in array
    else                                    // else read block failed
    { 
      print_Str("\n\r >> sd_ReadSingleBlock() returned ");
      if (errRSB & R1_ERROR)                // if failed with R1 error
      {
        print_Str("R1 error: ");            
        sd_PrintR1(errRSB);                 // then print the R1 error
      }
      else                                  // else failed with non-R1 error
      { 
        print_Str(" error ");               
        sd_PrintReadError(errRSB);          // so print the Read Error.
      }
    }
    
    #endif
    //
    // END TEST                                TEST_READ_AND_PRINT_SINGLE_BLOCK
    // ------------------------------------------------------------------------
    

    // ------------------------------------------------------------------------
    // BEGIN TEST                           TEST_READ_AND_PRINT_MULTIPLE_BLOCKS
    // 
    #if TEST_READ_AND_PRINT_MULTIPLE_BLOCKS

    uint16_t errPMB;   

    // print multiple blocks. Must determine if byte or block addressable.
    if (ctv.type == SDHC) 
      errPMB = sd_PrintMultipleBlocks(START_BLK_ADDR_PMB, NUM_OF_BLKS_PMB);
    else
      errPMB = sd_PrintMultipleBlocks((uint64_t)START_BLK_ADDR_PMB * BLOCK_LEN,
                                      NUM_OF_BLKS_PMB);
    
    // If an error was returned determine type and print it. 
    if (errPMB != READ_SUCCESS)
    { 
      print_Str("\n\r >> sd_PrintMultipleBlocks() returned ");
      if (errPMB & R1_ERROR)                // R1 error returned
      {
        print_Str("R1 error: ");
        sd_PrintR1(errPMB);
      }
      else                                  // non-R1 error returned
      { 
        print_Str(" error ");              
        sd_PrintReadError(errPMB);
      }   
    }

    #endif
    //
    // END TEST                             TEST_READ_AND_PRINT_MULTIPLE_BLOCKS
    // ------------------------------------------------------------------------


    // ------------------------------------------------------------------------
    // BEGIN TEST                                 TEST_ERASE_WRITE_SINGLE_BLOCK
    //
    #if TEST_ERASE_WRITE_SINGLE_BLOCK

    uint8_t  dataArrWSB[BLOCK_LEN] = WRITE_STR_WSB;   // data to be written
    uint8_t  blckArrWSB[BLOCK_LEN];         // array to store data read in
    uint16_t errWSB;                        // for returned error

    print_Str("\n\n\r >> Erasing Block ");
    print_Dec(BLK_ADDR_WSB);

    //
    // First erase the data block. Use erase start block = erase end block.
    // Must determine whether to use block or byte addressing.
    //
    if (ctv.type == SDHC)
      errWSB = sd_EraseBlocks(BLK_ADDR_WSB, BLK_ADDR_WSB);
    else
      errWSB = sd_EraseBlocks((uint64_t)BLK_ADDR_WSB * BLOCK_LEN, 
                              (uint64_t)BLK_ADDR_WSB * BLOCK_LEN);
    
    if (errWSB != ERASE_SUCCESS)         // if erase was not successful
    {
      print_Str("\n\r >> sd_EraseBlocks() returned ");
      if (errWSB & R1_ERROR)
      {
        print_Str("R1 error: ");
        sd_PrintR1(errWSB);
      }
      print_Str(" error "); 
      sd_PrintEraseError(errWSB);
    }

    print_Str("\n\r >> Reading and printing data from block "); 
    print_Dec(BLK_ADDR_WSB);
    print_Str(" to confirm data has been erased. All bytes should be 00.");
    
    // read in single block regardless of success/failure or erase.
    if (ctv.type == SDHC) 
      errWSB = sd_ReadSingleBlock(BLK_ADDR_WSB, blckArrWSB);
    else
      errWSB = sd_ReadSingleBlock((uint64_t)BLK_ADDR_WSB * BLOCK_LEN, 
                                  blckArrWSB);

    if (errWSB == READ_SUCCESS)             // if block read was successful
      sd_PrintSingleBlock(blckArrWSB);      // then print block loaded in array
    else                                    // else read block failed
    { 
      print_Str("\n\r >> sd_ReadSingleBlock() returned ");
      if (errWSB & R1_ERROR)                // if failed with R1 error
      {
        print_Str("R1 error: ");            
        sd_PrintR1(errWSB);                 // the print the R1 error
      }
      else                                  // else failed with non-R1 error
      { 
        print_Str(" error ");               
        sd_PrintReadError(errWSB);          // so print the Read Error.
      }
    }

    // write to data block regardless of success/failure for erase or read
    print_Str("\n\n\n\r >> Writing data to block "); 
    print_Dec(BLK_ADDR_WSB);
    if (ctv.type == SDHC) 
      errWSB = sd_WriteSingleBlock(BLK_ADDR_WSB, dataArrWSB);
    else
      errWSB = sd_WriteSingleBlock((uint64_t)BLK_ADDR_WSB * BLOCK_LEN, 
                                    dataArrWSB);

    if (errWSB != WRITE_SUCCESS)
    { 
      print_Str("\n\r >> sd_WriteSingleBlock() returned ");
      if (errWSB & R1_ERROR)                // if R1 error
      {
        print_Str("R1 error: ");
        sd_PrintR1(errWSB);
      }
      else                                  // else non-R1 error
      { 
        print_Str("error "); 
        sd_PrintWriteError(errWSB);
        
        //
        // Get the R2 (SEND_STATUS) response if the WRITE ERROR TOKEN was
        // returned by the card while writing to the block. R2 response is two
        // bytes, with first byte being the same as the R1 response.
        //
        if ((errWSB & WRITE_ERROR_TKN_RECEIVED) == WRITE_ERROR_TKN_RECEIVED)
        {
          print_Str("\n\r >> WRITE ERROR TOKEN returned. "
                    "Getting R2 response.");
          
          CS_ASSERT;             
          sd_SendCommand(SEND_STATUS, 0);
          uint16_t r2 = sd_GetR1();         // The first byte of R2 is R1
          r2 <<= 8;
          r2 |= sd_ReceiveByteFromSD();
          CS_DEASSERT;
          print_Str("\n\r >> R2 Response = 0x");
          print_Hex(r2);
        }
      }
    }
    else    // Verify block write by reading in and printing the block contents
    {
      print_Str("\n\r >> Reading and printing data in block "); 
      print_Dec(BLK_ADDR_WSB);
      print_Str(" to confirm data has successfully been written.");
      
      // read in block
      if (ctv.type == SDHC)
        errWSB = sd_ReadSingleBlock(BLK_ADDR_WSB, blckArrWSB);
      else
        errWSB = sd_ReadSingleBlock((uint64_t)BLK_ADDR_WSB * BLOCK_LEN, 
                                    blckArrWSB);

      if (errWSB == READ_SUCCESS)           // if block read was successful
        sd_PrintSingleBlock(blckArrWSB);    // then print block loaded in array
      else                                  // else read block failed
      { 
        print_Str("\n\r >> sd_ReadSingleBlock() returned ");
        if (errWSB & R1_ERROR)              // if failed with R1 error
        {
          print_Str("R1 error: ");            
          sd_PrintR1(errWSB);               // the print the R1 error
        }
        else                                // else failed with non-R1 error
        { 
          print_Str(" error ");               
          sd_PrintReadError(errWSB);        // so print the Read Error.
        }
      }
    }

    #endif
    //
    // END TEST                                   TEST_ERASE_WRITE_SINGLE_BLOCK
    // ------------------------------------------------------------------------


    // ------------------------------------------------------------------------
    // BEGIN TEST                                        TEST_COPY_SINGLE_BLOCK
    //
    #if TEST_COPY_SINGLE_BLOCK

    uint8_t  blckArrCSB[BLOCK_LEN];         // array for source block contents
    uint16_t errCSB;

    print_Str("\n\n\r >> Read and print contents of destination block "
              "before copying."
              "\n\r >> Destination Block Number: "); 
    print_Dec(DEST_BLK_CSB);
    
    // read in initial contents of destination block
    if (ctv.type == SDHC) 
      errCSB = sd_ReadSingleBlock(DEST_BLK_CSB, blckArrCSB);
    else
      errCSB = sd_ReadSingleBlock((uint64_t)DEST_BLK_CSB * BLOCK_LEN, 
                                  blckArrCSB);

    if (errCSB == READ_SUCCESS)             // if block read was successful
        sd_PrintSingleBlock(blckArrCSB);    // then print block loaded in array
    else                                    // else read block failed
    { 
      print_Str("\n\r >> sd_ReadSingleBlock() returned ");
      if (errCSB & R1_ERROR)                // if failed with R1 error
      {
        print_Str("R1 error: ");            
        sd_PrintR1(errCSB);                 // the print the R1 error
      }
      else                                  // else failed with non-R1 error
      { 
        print_Str(" error ");               
        sd_PrintReadError(errCSB);          // so print the Read Error.
      }
    }

    // read in and print source data block
    print_Str("\n\n\r >> Read in and print contents of source block."
              "\n\r >> Source Block Number: "); 
    print_Dec(SOURCE_BLK_CSB);

    // read in contents of source block
    if (ctv.type == SDHC) 
      errCSB = sd_ReadSingleBlock(SOURCE_BLK_CSB, blckArrCSB);
    else
      errCSB = sd_ReadSingleBlock((uint64_t)SOURCE_BLK_CSB * BLOCK_LEN, 
                                  blckArrCSB);

    if (errCSB == READ_SUCCESS)             // if block read was successful
        sd_PrintSingleBlock(blckArrCSB);    // then print block loaded in array
    else                                    // else read block failed
    { 
      print_Str("\n\r >> sd_ReadSingleBlock() returned ");
      if (errCSB & R1_ERROR)                // if failed with R1 error
      {
        print_Str("R1 error: ");            
        sd_PrintR1(errCSB);                 // the print the R1 error
      }
      else                                  // else failed with non-R1 error
      { 
        print_Str(" error ");               
        sd_PrintReadError(errCSB);          // so print the Read Error.
      }
    }

    // copy source block's contents to destination block.
    print_Str("\n\n\r >> Copying source block to destination block.");
    if (ctv.type == SDHC) 
      errCSB = sd_WriteSingleBlock(DEST_BLK_CSB, blckArrCSB);
    else
      errCSB = sd_WriteSingleBlock(DEST_BLK_CSB * BLOCK_LEN, blckArrCSB);

    if (errCSB != WRITE_SUCCESS)       // if data write was not successful
    { 
      print_Str("\n\r >> sd_WriteSingleBlock() returned ");
      if (errCSB & R1_ERROR)
      {
        print_Str("R1 error: ");
        sd_PrintR1(errCSB);
      }
      else 
      { 
        print_Str("error "); 
        sd_PrintWriteError(errCSB);

        //
        // Get the R2 (SEND_STATUS) response if the WRITE ERROR TOKEN was
        // returned by the card while writing to the block. R2 response is two
        // bytes, with first byte being the same as the R1 response.
        //
        if ((errCSB & WRITE_ERROR_TKN_RECEIVED) == WRITE_ERROR_TKN_RECEIVED)
        {
          print_Str("\n\r >> WRITE ERROR TOKEN returned. "
                    "Getting R2 response.");
          CS_ASSERT;             
          sd_SendCommand(SEND_STATUS,0);
          uint16_t r2 = sd_GetR1();         // The first byte of R2 is R1
          r2 <<= 8;
          r2 |= sd_ReceiveByteFromSD();
          CS_DEASSERT;
          print_Str("\n\r >> R2 Response = ");
          print_Hex(r2);
        }
      }
    }
    else    // Verify block write by reading in and printing the block contents
    {
      print_Str("\n\n\r >> Read destination block after copying contents."); 
      print_Str("\n\r >> Destination Block Number: "); 
      print_Dec(DEST_BLK_CSB);
      
      // read in contents of destination block, post copy.
      if (ctv.type == SDHC)
        errCSB = sd_ReadSingleBlock(DEST_BLK_CSB, blckArrCSB);
      else
        errCSB = sd_ReadSingleBlock(DEST_BLK_CSB * BLOCK_LEN, blckArrCSB);

      if (errCSB == READ_SUCCESS)           // if block read was successful
          sd_PrintSingleBlock(blckArrCSB);  // then print block loaded in array
      else                                  // else read block failed
      { 
        print_Str("\n\r >> sd_ReadSingleBlock() returned ");
        if (errCSB & R1_ERROR)              // if failed with R1 error
        {
          print_Str("R1 error: ");            
          sd_PrintR1(errCSB);               // the print the R1 error
        }
        else                                // else failed with non-R1 error
        { 
          print_Str(" error ");               
          sd_PrintReadError(errCSB);        // so print the Read Error.
        }
      }
    }

    #endif
    //
    // END TEST                                          TEST_COPY_SINGLE_BLOCK
    // ------------------------------------------------------------------------


    // ------------------------------------------------------------------------
    // BEGIN TEST                              TEST_WRITE_ERASE_MULTIPLE_BLOCKS
    //
    #if TEST_ERASE_WRITE_MULTIPLE_BLOCKS

    uint8_t  dataArrWMB[BLOCK_LEN] = WRITE_STR_WMB;   // data to be written
    uint64_t endEraseBlkAddr = START_BLK_ADDR_WMB + NUM_OF_BLKS_WMB - 1;
    uint16_t errWMB;

    // erase multiple blocks
    print_Str("\n\n\r >> Erasing blocks "); 
    print_Dec(START_BLK_ADDR_WMB);
    print_Str(" to ");
    print_Dec(endEraseBlkAddr);

    if (ctv.type == SDHC)
      errWMB = sd_EraseBlocks(START_BLK_ADDR_WMB, endEraseBlkAddr);
    else
      errWMB = sd_EraseBlocks((uint64_t)START_BLK_ADDR_WMB * BLOCK_LEN, 
                               endEraseBlkAddr * BLOCK_LEN);
    
    if (errWMB != ERASE_SUCCESS)         // if erase failed
    {
      print_Str("\n\r >> sd_EraseBlocks() returned ");
      if (errWMB & R1_ERROR)
      {
        print_Str("R1 error: ");
        sd_PrintR1(errWMB);
      }
      print_Str("error "); 
      sd_PrintEraseError(errWMB);
    }

    // Print Multiple Blocks
    print_Str("\n\r >> Printing blocks "); 
    print_Dec(START_BLK_ADDR_WMB);
    print_Str(" to ");
    print_Dec(endEraseBlkAddr);
    print_Str(" after erasing. All data bytes should be set to 00.");
    if (ctv.type == SDHC)
      errWMB = sd_PrintMultipleBlocks(START_BLK_ADDR_WMB, NUM_OF_BLKS_WMB);
    else 
      errWMB = sd_PrintMultipleBlocks((uint64_t)START_BLK_ADDR_WMB * BLOCK_LEN,
                                       NUM_OF_BLKS_WMB);
    
    if (errWMB != READ_SUCCESS)             // if print multiple blocks failed
    { 
      print_Str("\n\r >> sd_PrintMultipleBlocks() returned ");
      if (errWMB & R1_ERROR)
      {
        print_Str("R1 error: ");
        sd_PrintR1(errWMB);
      }
      else 
      { 
        print_Str(" error "); 
        sd_PrintReadError(errWMB);
      } 
    }
    
    do
    {
      print_Str("\n\n\n\r ** Press enter/return to continue with data write.");
    } while(usart_Receive() != '\r');
      

    // Write Multiple Blocks
    print_Str("\n\r >> Writing data to blocks "); 
    print_Dec(START_BLK_ADDR_WMB);
    print_Str(" to ");
    print_Dec(endEraseBlkAddr);
    
    errWMB = sd_WriteMultipleBlocks(START_BLK_ADDR_WMB, NUM_OF_BLKS_WMB, 
                                    dataArrWMB);
    if (errWMB != WRITE_SUCCESS)       // if write multiple blocks failed
    { 
      print_Str("\n\r >> sd_WriteMultipleBlocks() returned ");
      if (errWMB & R1_ERROR)                // then if failed with R1 error
      {
        print_Str("R1 error: ");
        sd_PrintR1(errWMB);
      } 
      else                                  // else if failed with non-R1 error
      { 
        print_Str("error "); 
        sd_PrintWriteError(errWMB);

        //
        // Get the R2 (SEND_STATUS) response if the WRITE ERROR TOKEN was
        // returned by the card while writing to the block. R2 response is two
        // bytes, with first byte being the same as the R1 response.
        //
        if ((errWMB & WRITE_ERROR_TKN_RECEIVED) == WRITE_ERROR_TKN_RECEIVED)
        {
          print_Str("\n\n\r >> WRITE_ERROR_TOKEN set."
                    "\n\r >> Getting STATUS (R2) response.");
          CS_ASSERT;             
          sd_SendCommand(SEND_STATUS, 0);
          uint16_t r2 = sd_GetR1();         // The first byte of R2 is R1
          r2 <<= 8;
          r2 |= sd_ReceiveByteFromSD();
          CS_DEASSERT;
          print_Str("\n\r >> R2 Response = ");
          print_Hex(r2);

          // Get Number of Well Written Blocks when WRITE ERROR TOKEN was RXd
          print_Str("\n\r >> Getting number of \"Well Written Blocks\".");
          uint32_t nwwb;
          errWMB = sd_GetNumOfWellWrittenBlocks(&nwwb);
          if (errWMB != READ_SUCCESS)
          { 
            print_Str("\n\r >> SD_GetNumberOfWellWritteBlocks() returned ");
            if (errWMB & R1_ERROR)
            {
              print_Str("R1 error: ");
              sd_PrintR1(errWMB);
            }
            else 
            { 
              print_Str("error "); 
              sd_PrintReadError(errWMB);
            }
            print_Str("\n\r Number of well written write blocks = ");
            print_Dec(nwwb);
          }
        }
      }
    }
        
    // post multi-block write READ and PRINT blocks.
    print_Str("\n\r >> Printing blocks "); 
    print_Dec(START_BLK_ADDR_WMB);
    print_Str(" to ");
    print_Dec(endEraseBlkAddr);
    print_Str(" after writing data.");

    if (ctv.type == SDHC)
      errWMB = sd_PrintMultipleBlocks(START_BLK_ADDR_WMB, NUM_OF_BLKS_WMB);
    else
      errWMB = sd_PrintMultipleBlocks((uint64_t)START_BLK_ADDR_WMB * BLOCK_LEN,
                                       NUM_OF_BLKS_WMB);
    
    if (errWMB != READ_SUCCESS)             // print multiple blocks failed
    { 
      print_Str("\n\r >> sd_PrintMultipleBlocks() returned ");
      if (errWMB & R1_ERROR)
      {
        print_Str("R1 error: ");
        sd_PrintR1(errWMB);
      }
      else 
      { 
        print_Str("error "); 
        sd_PrintReadError(errWMB);
      }
    }

    #endif
    //
    // END TEST                                TEST_ERASE_WRITE_MULTIPLE_BLOCKS
    // ------------------------------------------------------------------------


    // ------------------------------------------------------------------------
    // BEGIN TEST                                 TEST_INTERACTIVE_USER_SECTION
    //
    #if TEST_INTERACTIVE_USER_SECTION

    uint64_t startBlckNumUIS;               // first block to print
    uint64_t numOfBlcksUIS;                 // number of blocks to print
    uint8_t  ansIUS;                        // answer
    uint16_t errIUS;

    //
    // loop that requests user to specify blocks to read in and then reads in
    // and prints the blocks by calling sd_PrintMultipleBlocks. The loop exits 
    // when user enters 'q' after reading in blocks.
    //
    do
    {
      // loop to get user input. Continues until user replies with 'y'
      do
      {
        print_Str("\n\n\n\r >> Enter Start Block\n\r");
        startBlckNumUIS = enterBlockNumber();
        print_Str("\n\r >> How many blocks do you want to print?\n\r");
        numOfBlcksUIS = enterBlockNumber();
        print_Str("\n\r >> You have selected to print "); 
        print_Dec(numOfBlcksUIS);
        print_Str(" blocks beginning at block number "); 
        print_Dec(startBlckNumUIS);
        print_Str("\n\r >> Is this correct? (y/n)");
        ansIUS = usart_Receive();
        usart_Transmit(ansIUS);
        print_Str("\n\r");
      }
      while (ansIUS != 'y');

      // print the blocks specified above.
      if (ctv.type == SDHC) 
        errIUS = sd_PrintMultipleBlocks(startBlckNumUIS, numOfBlcksUIS);
      else
        errIUS = sd_PrintMultipleBlocks(startBlckNumUIS * BLOCK_LEN, numOfBlcksUIS);
      
      if (errIUS != READ_SUCCESS)
      { 
        print_Str("\n\r >> sd_PrintMultipleBlocks() returned ");
        if (errIUS & R1_ERROR)
        {
          print_Str("R1 error: ");
          sd_PrintR1(errIUS);
        }
        else 
        { 
          print_Str(" error "); 
          sd_PrintReadError(errIUS);
        }
      }

      print_Str("\n\n\r >> Press 'q' to quit: ");
      ansIUS = usart_Receive();
      usart_Transmit(ansIUS);
    }
    while (ansIUS != 'q');

    #endif
    //
    // END TEST                                   TEST_INTERACTIVE_USER_SECTION
    // ------------------------------------------------------------------------


    // ------------------------------------------------------------------------
    // BEGIN TEST                                          TEST_MEMORY_CAPACITY
    //
    #if TEST_MEMORY_CAPACITY

    print_Str("\n\n\n\r Memory capacity = ");
    print_Dec(sd_GetCardByteCapacity(&ctv));
    print_Str(" Bytes");

    #endif
    //
    // END TEST                                            TEST_MEMORY_CAPACITY
    // ------------------------------------------------------------------------



    // ------------------------------------------------------------------------
    // BEGIN TEST                                 TEST_FIND_NONZERO_DATA_BLOCKS
    //
    #if TEST_FIND_NONZERO_DATA_BLOCKS
    
    print_Str("\n\n\r\r sd_FindNonZeroDataBlockNums() \n\r");
    if (ctv.type == SDHC) 
      sd_FindNonZeroDataBlockNums(START_BLK_ADDR_FNZDB, END_BLK_ADDR_FNZDB);
    else
      sd_FindNonZeroDataBlockNums((uint64_t)START_BLK_ADDR_FNZDB * BLOCK_LEN, 
                                  (uint64_t)END_BLK_ADDR_FNZDB * BLOCK_LEN);
    print_Str("\n\r Done\n\r");

    #endif
    //
    // END TEST                                   TEST_FIND_NONZERO_DATA_BLOCKS
    // ------------------------------------------------------------------------
  }

  // This is just something to do after SD card testing has completed.
  while (1)
    usart_Transmit(usart_Receive());
  return 0;
}
// END MAIN()

//
// LOCAL FUNCTION - gets and returns block number for user input section.
//
#if TEST_INTERACTIVE_USER_SECTION

uint32_t enterBlockNumber()
{
  uint8_t  decDigit;
  uint8_t  asciiChar;
  uint32_t blkNum = 0;
  uint8_t  radix = 10;                      // decimal radix

  asciiChar = usart_Receive();
  
  while (asciiChar != '\r')
  {
    if (asciiChar >= '0' && asciiChar <= '9') // if decimal ascii char entered
    {
      decDigit = asciiChar - '0';           // convert ascii to decimal digit
      blkNum *= radix;
      blkNum += decDigit;
    }
    else if (asciiChar == BACKSPACE)        // if backspace on keyboard entered
    {
      print_Str("\b ");                     // print backspace and space chars
      blkNum = blkNum / radix;       // reduce current blkNum by factor of 10
    }
    print_Str("\r");
    print_Dec(blkNum);
    
    if (blkNum >= MAX_BLOCK_NUM_32_BIT)
    {
      blkNum = 0;                           // reset block number
      print_Str("\n\rblock number too large. Enter value < ");
      print_Dec(MAX_BLOCK_NUM_32_BIT);
      print_Str("\n\r");  
    }
    asciiChar = usart_Receive();
  }
  return blkNum;
}

#endif
