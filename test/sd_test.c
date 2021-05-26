/*
 * File       : SD_TEST.C
 * Author     : Joshua Fain
 * Target     : ATMega1280
 * Compiler   : AVR-GCC 9.3.0
 * Downloader : AVRDUDE 6.3
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020, 2021
 *
 * Test file to demonstrate the implementation of several functions defined in
 * the SD_SPI_XXXX files. main is organized in various sections that can be 
 * enabled independently using the SD Card Feature Testing Macros provided in
 * this file. 
 */

#include <avr/io.h>
#include "usart0.h"
#include "prints.h"
#include "sd_spi_base.h"
#include "sd_spi_rwe.h"
#include "sd_spi_misc.h"

#define SD_CARD_INIT_ATTEMPTS_MAX      5
#define MAX_DATA_BYTES_32_BIT          2147483648 
#define MAX_BLOCK_NUM_32_BIT           (MAX_DATA_BYTES_32_BIT / BLOCK_LEN)
#define BACKSPACE                      127  // used for keyboard backspace here

// 
// SD Card Feature Testing Macros
//
// This first set of macro definitions will enable the various test sections in
// main below. The macro section below this, will include the relevant macro
// definitions as required according to the test sections that are enabled in 
// this section. A description of what each section does is provided above its
// set of enabled macro paramters provided in the next section.
//
#define READ_IN_AND_PRINT_SINGLE_BLOCK_TESTING     0
#define PRINT_MULTIPLE_BLOCKS_TESTING              0
#define WRITE_ERASE_SINGLE_BLOCK_TESTING           0
#define COPY_SINGLE_BLOCK_TESTING                  0
#define WRITE_ERASE_MULTIPLE_BLOCKS_TESTING        0
#define USER_INPUT_SECTION_TESTING                 0
#define MEMORY_CAPACITY_TESTING                    1
#define FIND_NONZERO_DATA_BLOCKS_TESTING           0

//
// ----------------------------------------------------------------------------
//                                       READ_IN_AND_PRINT_SINGLE_BLOCK_TESTING
// 
// The corresponding section in main demos the functions sd_ReadSingleBlock()
// and sd_PrintSingleBlock(). This section will use sd_ReadSingleBlock() to 
// read the contents of the block at the address specified by BLK_ADDR_RSB into
// an array. This array will then be passed to sd_PrintSingleBlock() which will
// print the block's contents to the screen. 
//
#if READ_IN_AND_PRINT_SINGLE_BLOCK_TESTING
#define BLK_ADDR_RSB         16384          // addr of blk to read in and print
#endif

// ----------------------------------------------------------------------------
//                                                PRINT_MULTIPLE_BLOCKS_TESTING
// 
// The corresponding section in main demos sd_PrintMultipleBlocks(). This will
// print the number of blocks specified by NUM_OF_BLKS_PMB, beginging at 
// START_BLK_ADDR_PMB. The function calls the READ_MULTIPLE_BLOCKS SD card 
// command.
//
#if PRINT_MULTIPLE_BLOCKS_TESTING
#define START_BLK_ADDR_PMB   20             // addr of first blk to print
#define NUM_OF_BLKS_PMB      2              // total num of blks to print
#endif

// ----------------------------------------------------------------------------
//                                             WRITE_ERASE_SINGLE_BLOCK_TESTING
//
// The corresponding section in main demos the functions sd_WriteSingleBlock(), 
// sd_EraseBlocks(), sd_ReadSingleBlock() and sd_printSingleBlocks(). This will
// will first erase the single block at the block address specified by 
// BLK_ADDR_WSB using sd_EraseBlocks(), then read in and print this block to 
// confirm it is erased using sd_ReadSingleBlock and sd_PrintSingleBlock(). 
// Next, it will call the sd_WriteSingleBlock() function to the data specified
// by WRITE_STR_WSB to the same block. Finally, it will read in and print the 
// block again to confirm the block write was successful.
//
#if WRITE_ERASE_SINGLE_BLOCK_TESTING
// string/data that will be written to the block
#define WRITE_STR_WSB        "Well Hi, I See you brought a PIE!!!"
#define BLK_ADDR_WSB         20            // block that will be written to
#endif

// ----------------------------------------------------------------------------
//                                                    COPY_SINGLE_BLOCK_TESTING
//
// The corresponding section in main will copy the contents in the block
// specified by SOURCE_BLK_CSB into the block specified by DEST_BLK_CSB. It
// does this by reading the contents at SOURCE_BLK_CSB into an array using 
// sd_ReadSingleBlock, and then passing a pointer to this array to 
// sd_WriteSingleBlock to write its contents to DEST_BLK_CSB.
//
#if COPY_SINGLE_BLOCK_TESTING
#define SOURCE_BLK_CSB       0    // block holding data that will be copied
#define DEST_BLK_CSB         20   // block where data will be copied to
#endif

// ----------------------------------------------------------------------------
//                                          WRITE_ERASE_MULTIPLE_BLOCKS_TESTING
//
// The corresponding section in main will demos sd_writeMultipleBlock, 
// sd_EraseBlocks, and sd_PrintMultipleBlocks. This section will first erase
// the multiple blocks using sd_EraseBlocks, then read-in and print the same
// blocks using sd_PrintMultipleBlocks to verify the blocks have been erased. 
// Next it will write to the multiple blocks. It should then do some error 
// handling if WRITE_ERROR_TKN_RECEIVED is returned, but this has not been 
// tested yet. Finally, it will read-in and print the same blocks again to show
// successful write. The data/string that will be written is the value of 
// WRITE_STR_WMB. This data/string will be copied to the beginning of each
// block written to. The total number of blocks to be erased and written to is
// given by NUM_OF_BLKS_WSB and begins at START_BLK_ADDR_WMB.
//
#if WRITE_ERASE_MULTIPLE_BLOCKS_TESTING
// Data (string) to be written to the block 
#define WRITE_STR_WMB        "Would you like to play a game???"
#define START_BLK_ADDR_WMB   20             // start block for erasing
#define NUM_OF_BLKS_WMB      2              // used for writing
#endif

// ----------------------------------------------------------------------------
//                                                   USER_INPUT_SECTION_TESTING 
//
// This section allows a user to interact with the function 
// sd_PrintMultipleBlocks, and no additional macros are needed for this. The 
// user is asked which block number they would like to print first, and then 
// how many blocks they would like to print. The sd_PrintMultipleBlocks 
// function is then called with these parameters and the blocks specified are 
// printed. This function uses the local function enterBlockNumber, to get the 
// user-entered block number.
//
#if USER_INPUT_SECTION_TESTING
// local function used to get block number from user
uint32_t enterBlockNumber();  
#endif

// ----------------------------------------------------------------------------
//                                                      MEMORY_CAPACITY_TESTING
//
// The section calls the function sd_GetCardByteCapacity which will calculate
// and return the card's data capacity in bytes. No additional macros are 
// needed for this function.

// ----------------------------------------------------------------------------
//                                             FIND_NONZERO_DATA_BLOCKS_TESTING
//
// The corresponding section in main will demo the function 
// sd_FindNonZeroDataBlockNums which will print the block address/number of any
// block in the range of START_BLK_ADDR_FNZDB to END_BLK_ADDR_FNZDB (inclusive)
// that has any non-zero data in it. This function was created to help locate 
// raw data on the SD card, but it is not fast in printing to the screen, so I
// recommend testing it over a small range of blocks first.
//
#if FIND_NONZERO_DATA_BLOCKS_TESTING
// block number range to search for non-zero data.
#define START_BLK_ADDR_FNZDB  0
#define END_BLK_ADDR_FNZDB    100
#endif

int main(void)                                        
{
  // Initialize usart
  usart_Init();

  //
  // SD CARD INITILIAIZATION
  //
  // The first step to using the AVR-SD Card modules is to initialize the card 
  // into SPI mode. This is done by calling the sd_spiModeInit function and 
  // passing it a pointer to a CTV instance. The members of the instance will 
  // be set by the initialization routine. The value of the 'type' member is
  // used to determine how the data blocks on the card will be addressed.
  //
  uint32_t initResp;
  CTV ctv;

  // Loop will continue until SD card init succeeds or max attempts reached.
  for (uint8_t att = 0; att < SD_CARD_INIT_ATTEMPTS_MAX; ++att)
  {
    print_Str("\n\n\r >> SD Card Initialization Attempt "); 
    print_Dec(att);
    initResp = sd_InitModeSPI(&ctv);        // init SD Card

    if (initResp != OUT_OF_IDLE)            // Fail to init if not OUT_OF_IDLE
    {    
      print_Str(": FAILED TO INITIALIZE SD CARD."
                " Initialization Error Response: "); 
      sd_PrintInitError(initResp);
      print_Str(" R1 Response: "); 
      sd_PrintR1(initResp);
    }
    else
    {   
      print_Str(": SD CARD INITIALIZATION SUCCESSFUL");
      break;
    }
  }

  if (initResp == OUT_OF_IDLE)              // initialization successful
  {      
    // ------------------------------------------------------------------------
    //                                   READ_IN_AND_PRINT_SINGLE_BLOCK_TESTING
    //
    #if READ_IN_AND_PRINT_SINGLE_BLOCK_TESTING

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
        sd_PrintR1(errRSB);                 // the print the R1 error
      }
      else                                  // else failed with non-R1 error
      { 
        print_Str(" error ");               
        sd_PrintReadError(errRSB);          // so print the Read Error.
      }
    }
    
    #endif 
    //                               END READ_IN_AND_PRINT_SINGLE_BLOCK_TESTING
    // ------------------------------------------------------------------------
    

    // ------------------------------------------------------------------------
    //                                            PRINT_MULTIPLE_BLOCKS_TESTING
    // 
    #if PRINT_MULTIPLE_BLOCKS_TESTING

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
    //                                        END PRINT_MULTIPLE_BLOCKS_TESTING
    // ------------------------------------------------------------------------


    // ------------------------------------------------------------------------
    //                                         WRITE_ERASE_SINGLE_BLOCK_TESTING
    //
    #if WRITE_ERASE_SINGLE_BLOCK_TESTING

    uint8_t  dataArrWSB[BLOCK_LEN] = WRITE_STR_WSB;   // data to be written
    uint8_t  blckArrWSB[BLOCK_LEN];         // array for read in and print
    uint16_t errWSB;                        // for returned error

    print_Str("\n\n\r Erasing Block ");
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
    
    if (errWSB != ERASE_SUCCESSFUL)         // if erase was not successful
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

    print_Str("\n\r Read and Print Block "); 
    print_Dec(BLK_ADDR_WSB);
    print_Str(" after erasing.");
    
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
    print_Str("\n\n\n\r Write to block "); 
    print_Dec(BLK_ADDR_WSB);
    if (ctv.type == SDHC) 
      errWSB = sd_WriteSingleBlock(BLK_ADDR_WSB, dataArrWSB);
    else
      errWSB = sd_WriteSingleBlock((uint64_t)BLK_ADDR_WSB * BLOCK_LEN, 
                                    dataArrWSB);

    if (errWSB != DATA_WRITE_SUCCESS)
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
          print_Str("\n\r WRITE ERROR TOKEN returned. "
                    "Getting R2 response.");
          
          CS_SD_LOW;             
          sd_SendCommand(SEND_STATUS,0);
          uint16_t r2 = sd_GetR1();         // The first byte of R2 is R1
          r2 <<= 8;
          r2 |= sd_ReceiveByteSPI();
          CS_SD_HIGH;
          print_Str("\n\r R2 Response = 0x");
          print_Hex(r2);
        }
      }
    }
    else    // Verify block write by reading in and printing the block contents
    {
      print_Str("\n\r Read and Print Block "); 
      print_Dec(BLK_ADDR_WSB);
      print_Str(" after writing");
      
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
    //                                     END WRITE_ERASE_SINGLE_BLOCK_TESTING
    // ------------------------------------------------------------------------


    // ------------------------------------------------------------------------
    //                                                COPY_SINGLE_BLOCK_TESTING
    //
    #if COPY_SINGLE_BLOCK_TESTING

    uint8_t  blckArrCSB[BLOCK_LEN];         // array for source block contents
    uint16_t errCSB;

    print_Str("\n\n\r Read in and print contents of destination block "
              "before copying."
              "\n\r Destination Block Number: "); 
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
    print_Str("\n\n\r Read in and print contents of source block."
              "\n\r Source Block Number: "); 
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
    print_Str("\n\n\r Copying source block to destination block.");
    if (ctv.type == SDHC) 
      errCSB = sd_WriteSingleBlock(DEST_BLK_CSB, blckArrCSB);
    else
      errCSB = sd_WriteSingleBlock(DEST_BLK_CSB * BLOCK_LEN, blckArrCSB);

    if (errCSB != DATA_WRITE_SUCCESS)       // if data write was not successful
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
          print_Str("\n\r WRITE ERROR TOKEN returned. "
                    "Getting R2 response.");
          CS_SD_LOW;             
          sd_SendCommand(SEND_STATUS,0);
          uint16_t r2 = sd_GetR1();         // The first byte of R2 is R1
          r2 <<= 8;
          r2 |= sd_ReceiveByteSPI();
          CS_SD_HIGH;
          print_Str("\n\r R2 Response = ");
          print_Hex(r2);
        }
      }
    }
    else    // Verify block write by reading in and printing the block contents
    {
      print_Str("\n\n\r Read destination block after copying contents."); 
      print_Str("\n\r Destination Block Number: "); 
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
    //                                            END COPY_SINGLE_BLOCK_TESTING
    // ------------------------------------------------------------------------



    // ------------------------------------------------------------------------
    //                                      WRITE_ERASE_MULTIPLE_BLOCKS_TESTING
    //
    #if WRITE_ERASE_MULTIPLE_BLOCKS_TESTING

    uint8_t  dataArrWMB[BLOCK_LEN] = WRITE_STR_WMB;   // data to be written
    uint64_t endEraseBlkAddr = START_BLK_ADDR_WMB + NUM_OF_BLKS_WMB + 1;
    uint16_t errWMB;

    // erase multiple blocks
    print_Str("\n\n\r ERASING BLOCKS ");
    if (ctv.type == SDHC)
      errWMB = sd_EraseBlocks(START_BLK_ADDR_WMB, endEraseBlkAddr);
    else
      errWMB = sd_EraseBlocks((uint64_t)START_BLK_ADDR_WMB * BLOCK_LEN, 
                               endEraseBlkAddr * BLOCK_LEN);
    
    if (errWMB != ERASE_SUCCESSFUL)         // if erase failed
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
    print_Str("\n\n\r PRINTING BLOCKS BEFORE WRITE ");
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

    // Write Multiple Blocks
    print_Str("\n\n\r WRITING BLOCKS ");
    errWMB = sd_WriteMultipleBlocks(START_BLK_ADDR_WMB, NUM_OF_BLKS_WMB, 
                                    dataArrWMB);
    if (errWMB != DATA_WRITE_SUCCESS)       // if write multiple blocks failed
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
          print_Str("\n\n\r WRITE_ERROR_TOKEN set."
                    "\n\r Getting STATUS (R2) response.");
          CS_SD_LOW;             
          sd_SendCommand(SEND_STATUS, 0);
          uint16_t r2 = sd_GetR1();         // The first byte of R2 is R1
          r2 <<= 8;
          r2 |= sd_ReceiveByteSPI();
          CS_SD_HIGH;
          print_Str("\n\r R2 Response = ");
          print_Hex(r2);

          // Get Number of Well Written Blocks when WRITE ERROR TOKEN was RXd
          print_Str("\n\r Getting Number of Well Written Blocks.");
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
    print_Str("\n\r PRINTING BLOCKS AFTER WRITE ");
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
    //                                  END WRITE_ERASE_MULTIPLE_BLOCKS_TESTING
    // ------------------------------------------------------------------------


    // ------------------------------------------------------------------------
    //                                               USER_INPUT_SECTION_TESTING
    //
    #if USER_INPUT_SECTION_TESTING

    uint64_t startBlckNumUIS;               // first block to print
    uint64_t numOfBlcksUIS;                 // number of blocks to print
    uint8_t  ansUIS;                        // answer
    uint16_t errUIS;

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
        print_Str("\n\n\n\rEnter Start Block\n\r");
        startBlckNumUIS = enterBlockNumber();
        print_Str("\n\rHow many blocks do you want to print?\n\r");
        numOfBlcksUIS = enterBlockNumber();
        print_Str("\n\rYou have selected to print "); 
        print_Dec(numOfBlcksUIS);
        print_Str(" blocks beginning at block number "); 
        print_Dec(startBlckNumUIS);
        print_Str("\n\rIs this correct? (y/n)");
        ansUIS = usart_Receive();
        usart_Transmit(ansUIS);
        print_Str("\n\r");
      }
      while (ansUIS != 'y');

      // print the blocks specified above.
      if (ctv.type == SDHC) 
        errUIS = sd_PrintMultipleBlocks(startBlckNumUIS, numOfBlcksUIS);
      else
        errUIS = sd_PrintMultipleBlocks(startBlckNumUIS * BLOCK_LEN, numOfBlcksUIS);
      
      if (errUIS != READ_SUCCESS)
      { 
        print_Str("\n\r >> sd_PrintMultipleBlocks() returned ");
        if (errUIS & R1_ERROR)
        {
          print_Str("R1 error: ");
          sd_PrintR1(errUIS);
        }
        else 
        { 
          print_Str(" error "); 
          sd_PrintReadError(errUIS);
        }
      }

      print_Str("\n\rPress 'q' to quit: ");
      ansUIS = usart_Receive();
      usart_Transmit(ansUIS);
    }
    while (ansUIS != 'q');

    #endif
    //                                           END USER_INPUT_SECTION_TESTING
    // ------------------------------------------------------------------------


    // ------------------------------------------------------------------------
    //                                                  MEMORY_CAPACITY_TESTING
    //
    #if MEMORY_CAPACITY_TESTING

    print_Str("\n\n\n\r Memory capacity = ");
    print_Dec(sd_GetCardByteCapacity(&ctv));
    print_Str(" Bytes");

    #endif
    //                                              END MEMORY_CAPACITY_TESTING
    // ------------------------------------------------------------------------



    // ------------------------------------------------------------------------
    //                                         FIND_NONZERO_DATA_BLOCKS_TESTING
    //
    #if FIND_NONZERO_DATA_BLOCKS_TESTING
    
    print_Str("\n\n\r\r sd_FindNonZeroDataBlockNums() \n\r");
    if (ctv.type == SDHC) 
      sd_FindNonZeroDataBlockNums(START_BLK_ADDR_FNZDB, END_BLK_ADDR_FNZDB);
    else
      sd_FindNonZeroDataBlockNums((uint64_t)START_BLK_ADDR_FNZDB * BLOCK_LEN, 
                                  (uint64_t)END_BLK_ADDR_FNZDB * BLOCK_LEN);
    print_Str("\n\r Done\n\r");

    #endif
    //                                     END FIND_NONZERO_DATA_BLOCKS_TESTING
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
#if USER_INPUT_SECTION_TESTING

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
