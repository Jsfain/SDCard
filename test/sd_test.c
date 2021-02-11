/*
 * File    : SD_TEST.C
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : MIT
 * Copyright (c) 2020
 *
 * Test file to demonstrate how to implement several of the functions defined
 * in the SD_SPI_XXX files.
 */

#include <stdint.h>
#include <avr/io.h>
#include "usart0.h"
#include "prints.h"
#include "spi.h"
#include "sd_spi_base.h"
#include "sd_spi_rwe.h"
#include "sd_spi_misc.h"

// local function
uint32_t enterBlockNumber();

int main(void)
{
  // Initializat usart and spi ports.
  usart_Init();
  spi_MasterInit();

  // --------------------------------------------------------------------------
  //                                                    SD CARD INITILIAIZATION
  //
  // The first step to using the AVR-SD Card modules is to initialize the card 
  // into SPI mode. This is done by calling the sd_spiModeInit() function and 
  // passing it a ptr to a CTV instance. The members of the instance will be
  // set by the initialization routine. In particular, how the 'type' member is
  // set will determine how the data blocks on the card will be addressed.

  uint32_t initResp;

  CTV ctv;

  // Attempt SD card initialization up to 5 times.
  for (uint8_t i = 0; i < 5; i++)
  {
    print_Str("\n\n\r >> SD Card Initialization Attempt "); 
    print_Dec(i);
    initResp = sd_InitModeSPI(&ctv);

    // Failed to initialize if initResp is not 0.
    if (initResp != 0)
    {    
      print_Str(": FAILED TO INITIALIZE SD CARD.");
      print_Str(" Initialization Error Response: "); 
      sd_printInitError (initResp);
      print_Str(", R1 Response: "); 
      sd_printR1(initResp);
    }
    else
    {   
      print_Str(": SD CARD INITIALIZATION SUCCESSFUL");
      break;
    }
  }
  //                                                END SD CARD INITILIAIZATION
  // --------------------------------------------------------------------------



  // initialization successful
  if (initResp == 0)
  {      
    
    // ------------------------------------------------------------------------
    //                                           READ IN AND PRINT SINGLE BLOCK
    // 
    // Demo sd_readSingleBlock() and sd_printSingleBlock(). This section will
    // use sd_readSingleBlock() to read a block at the address specified into
    // the an array. This array will then be passed to sd_printSingleBlock()
    // which will print the block's contents to the screen.
    
    // set to 0 to skip this section
    #if 1

    uint32_t blckNum1 = 8192;               // block number to read in
    uint8_t  blckArr1[512];                 // array to hold block contents
    uint16_t err1;                           

    // read in the single block into the array
    if (ctv.type == SDHC)
      err1 = sd_readSingleBlock(blckNum1, blckArr1);
    else
      err1 = sd_readSingleBlock(blckNum1 * BLOCK_LEN, blckArr1);
    
    if (err1 != READ_SUCCESS)
    { 
      print_Str("\n\r >> sd_readSingleBlock() returned ");
      if (err1 & R1_ERROR)
      {
        print_Str("R1 error: ");
        sd_printR1(err1);
      }
      else 
      { 
        print_Str(" error "); 
        sd_printReadError(err1);
      }
    }

    // print block just loaded into the array.
    else
      sd_printSingleBlock(blckArr1);
    
    #endif 
    //                                       END READ IN AND PRINT SINGLE BLOCK
    // ------------------------------------------------------------------------
    

    // ------------------------------------------------------------------------
    //                                                    PRINT MULTIPLE BLOCKS
    // 
    // Demo sd_printMultipleBlocks(). This will print multiple, consecutive 
    // SD card blocks beginging at blckNum. The number of blocks printed is
    // specified by numOfBlcks. The function calls the READ_MULTIPLE_BLOCKS 
    // SD card command.

    // set to 0 to skip this section
    #if 0

    uint32_t numOfBlcks2 = 2;          // total number of blocks to print
    uint32_t blckNum2 = 20;            // first block to be printed
    uint16_t err2;   

    // print the multiple blocks
    if (ctv.type == SDHC) 
      err2 = sd_printMultipleBlocks(blckNum2, numOfBlcks2);
    else
      err2 = sd_printMultipleBlocks(blckNum2 * BLOCK_LEN, numOfBlcks2);
    
    if (err2 != READ_SUCCESS)
    { 
      print_Str("\n\r >> sd_printMultipleBlocks() returned ");
      if (err2 & R1_ERROR)
      {
        print_Str("R1 error: ");
        sd_printR1(err2);
      }
      else 
      { 
        print_Str(" error "); 
        sd_printReadError(err2);
      }   
    }

    #endif 
    //                                                END PRINT MULTIPLE BLOCKS
    // ------------------------------------------------------------------------


    // ------------------------------------------------------------------------
    //                                             WRITE and ERASE SINGLE BLOCK
    //
    // Demo sd_writeSingleBlock(), sd_eraseBlocks(), and sd_readSingleBlock()
    // and sd_printSingleBlocks(). This demo will first erase a single 
    // block using sd_eraseBlocks(), then read in and print the block to 
    // confirm it is erased using sd_readSingleBlock and sd_printSingleBlock().
    // It will then call the sd_writeSingleBlock() function to write data to
    // the block. Finally, it will read in and print the block again to confirm
    // the block was successfully written to. 

    // set to 0 to skip this section
    #if 1

    // Data (string) to be written to the block 
    uint8_t  dataArr3[BLOCK_LEN] = "Well Hi, I See you brought a PIE!!!";
    
    uint32_t blckNum3 = 20;                 // block to be written to
    uint8_t  blckArr3[512];                 // array to hold block contents
    uint16_t err3;

    print_Str("\n\r Erasing Block ");
    print_Dec(blckNum3);

    // erase single block (i.e. erase start block = erase end block)
    if (ctv.type == SDHC)
      err3 = sd_eraseBlocks(blckNum3, blckNum3);
    else
      err3 = sd_eraseBlocks(blckNum3 * BLOCK_LEN, blckNum3 * BLOCK_LEN);
    
    if (err3 != ERASE_SUCCESSFUL)
    {
      print_Str("\n\r >> sd_eraseBlocks() returned ");
      if (err3 & R1_ERROR)
      {
        print_Str("R1 error: ");
        sd_printR1(err3);
      }
      print_Str(" error "); 
      sd_printEraseError(err3);
    }

    // read in and print data block
    print_Str("\n\r Read and Print Block "); 
    print_Dec(blckNum3);
    print_Str(" after erasing.");
    
    if (ctv.type == SDHC) 
      err3 = sd_readSingleBlock(blckNum3, blckArr3);
    else
      err3 = sd_readSingleBlock(blckNum3 * BLOCK_LEN, blckArr3);

    if (err3 != READ_SUCCESS)
    { 
      print_Str("\n\r >> sd_readSingleBlock() returned ");
      if (err3 & R1_ERROR)
      {
        print_Str("R1 error: ");
        sd_printR1(err3);
      }
      else 
      { 
        print_Str(" error "); 
        sd_printReadError(err3);
      }
    }
    // print data block just loaded into the array.
    else
      sd_printSingleBlock(blckArr3);

    // write to data block
    print_Str("\n\r Write to block "); 
    print_Dec(blckNum3);
    if (ctv.type == SDHC) 
      err3 = sd_writeSingleBlock(blckNum3, dataArr3);
    else
      err3 = sd_writeSingleBlock(blckNum3 * BLOCK_LEN, dataArr3);

    if (err3 != DATA_ACCEPTED_TOKEN_RECEIVED)
    { 
      print_Str("\n\r >> sd_writeSingleBlock() returned ");
      if (err3 & R1_ERROR)
      {
        print_Str("R1 error: ");
        sd_printR1(err3);
      }
      else 
      { 
        print_Str("error "); 
        sd_printWriteError(err3);

        // Get the R2 (SEND_STATUS) response if the Write Error Token
        // was returned by the card while writing to the block.
        // May convert this to a function. 
        if ((err3 & WRITE_ERROR_TOKEN_RECEIVED) == WRITE_ERROR_TOKEN_RECEIVED)
        {
          print_Str("\n\r WRITE ERROR TOKEN returned.");
          print_Str("Getting R2 response.");
          
          CS_SD_LOW;             
          sd_sendCommand(SEND_STATUS,0);
          // The first byte of R2 is R1
          uint16_t r2 = sd_getR1(); 
          r2 <<= 8;
          r2 |= sd_receiveByteSPI();
          CS_SD_HIGH;
          print_Str("\n\r R2 Response = ");
          print_Dec(r2);
        }
      }
    }

    // Verify data has been written to the block 
    // by reading in and printing the block
    else 
    {
      print_Str("\n\r Read and Print Block "); 
      print_Dec(blckNum3);
      print_Str(" after writing");

      if (ctv.type == SDHC)
        err3 = sd_readSingleBlock(blckNum3, blckArr3);
      else
        err3 = sd_readSingleBlock(blckNum3 * BLOCK_LEN, blckArr3);

      if (err3 != READ_SUCCESS)
      { 
        print_Str("\n\r >> sd_readSingleBlock() returned ");
        if (err3 & R1_ERROR)
        {
          print_Str("R1 error: ");
          sd_printR1(err3);
        }
        else 
        { 
          print_Str(" error "); 
          sd_printReadError(err3);
        }
      }
      // print the single data block that was just read in.
      else
        sd_printSingleBlock(blckArr3);
    }

    #endif
    //                                         END WRITE and ERASE SINGLE BLOCK
    // ------------------------------------------------------------------------


    // ------------------------------------------------------------------------
    //                                                        COPY SINGLE BLOCK
    //
    // Reads data into an array using sd_readSingleBlock() then writes this
    // to another block using sd_writeSingleBlock().

    // set to 0 to skip this section
    #if 0

    // block to be written.
    uint32_t srcBlck4  = 0;        // block holding data that will be copied
    uint32_t destBlck4 = 20;       // block where data will be copied to
    uint8_t  blckArr4[512];        // array to hold block contents */
    uint16_t err4;

    // read in and print destination block before copying source contents.
    print_Str("\n\n\r Read in and print contents of destination block");
    print_Str(" before copying.");
    print_Str("\n\r Destination Block Number: "); 
    print_Dec(destBlck4);
    
    if (ctv.type == SDHC) 
      err4 = sd_readSingleBlock(destBlck4, blckArr4);
    else
      err4 = sd_readSingleBlock(destBlck4 * BLOCK_LEN, blckArr4);

    if (err4 != READ_SUCCESS)
    { 
      print_Str("\n\r >> sd_readSingleBlock() returned ");
      if (err4 & R1_ERROR)
      {
        print_Str("R1 error: ");
        sd_printR1(err4);
      }
      else 
      { 
        print_Str(" error "); 
        sd_printReadError(err4);
      }
    }
    // print data block just loaded into the array.
    else
      sd_printSingleBlock(blckArr4);


    // read in and print source data block
    print_Str("\n\n\r Read in and print contents of source block.");
    print_Str("\n\r Source Block Number: "); 
    print_Dec(srcBlck4);
    
    if (ctv.type == SDHC) 
      err4 = sd_readSingleBlock(srcBlck4, blckArr4);
    else
      err4 = sd_readSingleBlock(srcBlck4 * BLOCK_LEN, blckArr4);

    if (err4 != READ_SUCCESS)
    { 
      print_Str("\n\r >> sd_readSingleBlock() returned ");
      if (err4 & R1_ERROR)
      {
        print_Str("R1 error: ");
        sd_printR1(err4);
      }
      else 
      { 
        print_Str(" error "); 
        sd_printReadError(err4);
      }
    }
    // print data block just loaded into the array.
    else
      sd_printSingleBlock(blckArr4);


    // copy source contents to destination block.
    print_Str("\n\n\r Copying source block to destination block.");

    if (ctv.type == SDHC) 
      err4 = sd_writeSingleBlock(destBlck4, blckArr4);
    else
      err4 = sd_writeSingleBlock(destBlck4 * BLOCK_LEN, blckArr4);

    if (err4 != DATA_ACCEPTED_TOKEN_RECEIVED)
    { 
      print_Str("\n\r >> sd_writeSingleBlock() returned ");
      if (err4 & R1_ERROR)
      {
        print_Str("R1 error: ");
        sd_printR1(err4);
      }
      else 
      { 
        print_Str("error "); 
        sd_printWriteError(err4);

        // Get the R2 (SEND_STATUS) response if the Write Error Token
        // was returned by the card while writing to the block.
        // May convert this to a function. 
        if ((err4 & WRITE_ERROR_TOKEN_RECEIVED) == WRITE_ERROR_TOKEN_RECEIVED)
        {
          print_Str("\n\r WRITE ERROR TOKEN returned.");
          print_Str("Getting R2 response.");
          
          CS_SD_LOW;             
          sd_sendCommand(SEND_STATUS,0);
          // The first byte of R2 is R1
          r2 = sd_getR1(); 
          r2 <<= 8;
          r2 |= sd_receiveByteSPI();
          CS_SD_HIGH;
          print_Str("\n\r R2 Response = ");
          print_Dec(r2);
        }
      }
    }

    // Verify data has been written to the block 
    // by reading in and printing the block
    else 
    {
      print_Str("\n\n\r Read destination block after copying contents."); 
      print_Str("\n\r Destination Block Number: "); 
      print_Dec(destBlck4);

      if (ctv.type == SDHC)
        err4 = sd_readSingleBlock(destBlck4, blckArr4);
      else
        err4 = sd_readSingleBlock(destBlck4 * BLOCK_LEN, blckArr4);

      if (err4 != READ_SUCCESS)
      { 
        print_Str("\n\r >> sd_readSingleBlock() returned ");
        if (err4 & R1_ERROR)
        {
          print_Str("R1 error: ");
          sd_printR1(err4);
        }
        else 
        { 
          print_Str(" error "); 
          sd_printReadError(err4);
        }
      }
      // print the single data block that was just read in.
      else
        sd_printSingleBlock(blckArr4);
    }

    #endif
    //                                         END WRITE and ERASE SINGLE BLOCK
    // ------------------------------------------------------------------------



    // ------------------------------------------------------------------------
    //                                          WRITE and ERASE MULTIPLE BLOCKS
    //
    // Demos sd_writeMultipleBlock(), sd_eraseBlocks(), and 
    // sd_printMultipleBlocks(). This demo will first erase the multiple blocks
    // using sd_eraseBlocks(), then read-in and print the same blocks using 
    // sd_printMultipleBlocks() to verify the blocks have been erased. Next it
    // will write to the multiple blocks. It should then do some error handling
    // if WRITE_BLOCK_TOKEN_RECIEVED is returned, but I haven't been able to
    // test this out yet. Finally, it will read-in and print the blocks again
    // to show successful write.

    // set to 0 to skip this section
    #if 0

    // Data (string) to be written to the block 
    uint8_t dataArr2[BLOCK_LEN] = "Would you like to play a game???"; 

    uint32_t startBlckNum5 = 20;                 // start block for erasing
    uint32_t endBlckNum5   = 22;                 // end block for erasing
    uint32_t numOfBlcks5   = 2;                  // used for writing
    uint16_t err5;
    
    // R2 response for SEND_STATUS. Used if write error.
    uint16_t r2;

    // number of well-written blocks. Used for multi-block write error.
    uint32_t nwwb5;
    
    // erase multiple blocks
    print_Str("\n\r ERASING BLOCKS ");
    if (ctv.type == SDHC)
      err5 = sd_eraseBlocks(startBlckNum5, endBlckNum5);
    else
      err5 = sd_eraseBlocks(startBlckNum5 * BLOCK_LEN, 
                            endBlckNum5 * BLOCK_LEN);
    
    if (err5 != ERASE_SUCCESSFUL)
    {
      print_Str("\n\r >> sd_eraseBlocks() returned ");
      if (err5 & R1_ERROR)
      {
        print_Str("R1 error: ");
        sd_printR1(err5);
      }
      print_Str("error "); 
      sd_printEraseError(err5);
    }


    // Print Multiple Blocks
    print_Str("\n\r PRINTING BLOCKS BEFORE WRITE ");
    if (ctv.type == SDHC)
      err5 = sd_printMultipleBlocks(startBlckNum5, numOfBlcks5);
    else 
      err5 = sd_printMultipleBlocks(startBlckNum5 * BLOCK_LEN, numOfBlcks5);
    
    if (err5 != READ_SUCCESS)
    { 
      print_Str("\n\r >> sd_printMultipleBlocks() returned ");
      if (err5 & R1_ERROR)
      {
        print_Str("R1 error: ");
        sd_printR1(err5);
      }
      else 
      { 
        print_Str(" error "); 
        sd_printReadError(err5);
      }
    }

    // Write Multiple Blocks
    print_Str("\n\r WRITING BLOCKS ");
    err5 = sd_writeMultipleBlocks(startBlckNum5, numOfBlcks5, dataArr2);
    if (err5 != DATA_ACCEPTED_TOKEN_RECEIVED)
    { 
      print_Str("\n\r >> sd_writeMultipleBlocks() returned ");
      if (err5 & R1_ERROR)
      {
        print_Str("R1 error: ");
        sd_printR1(err5);
      }
      else 
      { 
        print_Str("error "); 
        sd_printWriteError(err5);

        // Get the R2 (SEND_STATUS) response if the Write Error Token
        // was returned by the card while writing to the block.
        if ((err5 & WRITE_ERROR_TOKEN_RECEIVED) == WRITE_ERROR_TOKEN_RECEIVED)
        {
          // Getting R2 response.  May make this a function.
          print_Str("\n\r WRITE_ERROR_TOKEN set.");
          print_Str("\n\r Getting STATUS (R2) response.");
          CS_SD_LOW;             
          sd_sendCommand(SEND_STATUS, 0);
          // The first byte of R2 is R1
          r2 = sd_getR1(); 
          r2 <<= 8;
          r2 |= sd_receiveByteSPI();
          CS_SD_HIGH;
          print_Str("\n\r R2 Response = ");
          print_Dec(r2);

          // Number of Well Written Blocks
          print_Str("\n\r Getting Number of Well Written Blocks.");
          
          err5 = sd_getNumOfWellWrittenBlocks (&nwwb5);
          if (err5 != READ_SUCCESS)
          { 
            print_Str("\n\r >> SD_GetNumberOfWellWritteBlocks() returned ");
            if (err5 & R1_ERROR)
            {
              print_Str("R1 error: ");
              sd_printR1(err5);
            }
            else 
            { 
              print_Str("error "); 
              sd_printReadError(err5);
            }
            print_Str("\n\r Number of well written write blocks = ");
            print_Dec(nwwb5);
          }
        }
      }
    }
    
    
    // post multi-block write READ amd PRINT blocks.
    print_Str("\n\r PRINTING BLOCKS AFTER WRITE ");
    if (ctv.type == SDHC)
      err5 = sd_printMultipleBlocks(startBlckNum5, numOfBlcks5);
    else
      err5 = sd_printMultipleBlocks(startBlckNum5 * BLOCK_LEN, numOfBlcks5);
    
    if (err5 != READ_SUCCESS)
    { 
      print_Str("\n\r >> sd_printMultipleBlocks() returned ");
      if (err5 & R1_ERROR)
      {
        print_Str("R1 error: ");
        sd_printR1(err5);
      }
      else 
      { 
        print_Str("error "); 
        sd_printReadError(err5);
      }
    }

    #endif
    //                                      END WRITE and ERASE MULTIPLE BLOCKS
    // ------------------------------------------------------------------------


    // ------------------------------------------------------------------------
    //                                                       USER INPUT SECTION
    //
    // This section allows a user to interact with the function 
    // sd_printMultipleBlocks(). The user is asked which block number they 
    // would like to print first, and then how many blocks they would like to 
    // print. The sd_printMultipleBlocks() function is then called with these
    // parameters and the blocks specified by the user are printed.

    // set to 0 to skip this section
    #if 0

    uint32_t startBlckNum6;                // first block to print
    uint32_t numOfBlcks6;                  // number of blocks to print
    uint8_t  ans6;                         // answer
    uint16_t err6;

    do
    {
      do
      {
        print_Str("\n\n\n\rEnter Start Block\n\r");
        startBlckNum6 = enterBlockNumber();
        print_Str("\n\rHow many blocks do you want to print?\n\r");
        numOfBlcks6 = enterBlockNumber();
        print_Str("\n\rYou have selected to print "); 
        print_Dec(numOfBlcks6);
        print_Str(" blocks beginning at block number "); 
        print_Dec(startBlckNum6);
        print_Str("\n\rIs this correct? (y/n)");
        ans6 = usart_Receive();
        usart_Transmit(ans6);
        print_Str("\n\r");
      }
      while (ans6 != 'y');

      // print the blocks specified above.
      if (ctv.type == SDHC) 
        err6 = sd_printMultipleBlocks(startBlckNum6, numOfBlcks6);
      else
        err6 = sd_printMultipleBlocks(startBlckNum6 * BLOCK_LEN, numOfBlcks6);
      
      if (err6 != READ_SUCCESS)
      { 
        print_Str("\n\r >> sd_printMultipleBlocks() returned ");
        if (err6 & R1_ERROR)
        {
          print_Str("R1 error: ");
          sd_printR1(err6);
        }
        else 
        { 
          print_Str(" error "); 
          sd_printReadError(err6);
        }
      }

      print_Str("\n\rPress 'q' to quit: ");
      ans6 = usart_Receive();
      usart_Transmit(ans6);
    }
    while (ans6 != 'q');

    #endif
    //                                                   END USER INPUT SECTION
    // ------------------------------------------------------------------------


    // ------------------------------------------------------------------------
    //                                                          MEMORY CAPACITY
    //
    // Get the total memory capacity of the card, in bytes. How this is
    // calculated depends on the card type (i.e. ctv.type) which is used to 
    // select the correct function.

    // set to 0 to skip this section
    #if 0

    print_Str("\n\n\n\r Memory capacity = ");
    if (ctv.type == SDHC) 
      print_Dec(sd_getMemoryCapacitySDHC ());
    else
      print_Dec(sd_getMemoryCapacitySDSC ());
    print_Str(" Bytes");

    #endif
    //                                                      END MEMORY CAPACITY
    // ------------------------------------------------------------------------



    // ------------------------------------------------------------------------
    //                                                FIND NON-ZERO DATA BLOCKS
    //
    // Uses the function sd_findNonZeroDataBlockNums() to print the block 
    // numbers of any that have non-zero data. I made this to help locate raw 
    // data on the SD card. This is function is not fast in printing to the 
    // screen, so I do not suggest using it over a large range of blocks.

    // set to 0 to skip this section
    #if 0
    
    // block number range to search for non-zero data.
    uint32_t startBlckNum = 5001;
    uint32_t endBlckNum = 8000;

    print_Str("\n\n\r\r sd_findNonZeroDataBlockNums() \n\r");
    if (ctv.type == SDHC) 
      sd_findNonZeroDataBlockNums(startBlckNum, endBlckNum);
    else
      sd_findNonZeroDataBlockNums(startBlckNum * BLOCK_LEN, 
                                  endBlckNum * BLOCK_LEN);
    print_Str("\n\r Done\n\r");

    #endif
    //                                                      END MEMORY CAPACITY
    // ------------------------------------------------------------------------

  }
  // End SD Card Testing


  // This is just something to do after SD card testing has completed.
  while (1)
    usart_Transmit(usart_Receive());
  
  
  return 0;
}
// END MAIN()


// local function for taking user input to specify a block
// number. If nothing is entered then the block number is 0.
uint32_t enterBlockNumber()
{
  uint8_t x;
  uint8_t c;
  uint32_t blckNum = 0;

  c = usart_Receive();
  
  while (c != '\r')
  {
    if (c >= '0' && c <= '9')
    {
      x = c - '0';
      blckNum = blckNum * 10;
      blckNum += x;
    }
    else if (c == 127) // backspace
    {
      print_Str("\b ");
      blckNum = blckNum / 10;
    }

    print_Str("\r");
    print_Dec(blckNum);
    
    if (blckNum >= 4194304)
    {
      blckNum = 0;
      print_Str("\n\rblock number is too large.");
      print_Str("\n\rEnter value < 4194304\n\r");
    }
    c = usart_Receive();
  }
  return blckNum;
}
