/******************************************************************************
* File    : SD_TEST.C
* Author  : Joshua Fain
* Target  : ATMega1280
* License : MIT
* Copyright (c) 2020
*
*
* DESCRIPTION:
* Test file to demonstrate how to implement several of the functions defined
* in the SD_SPI_XXX files.
******************************************************************************/



#include <stdint.h>
#include <avr/io.h>
#include "usart.h"
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
  usart_init();
  spi_masterInit();

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
    print_str ("\n\n\r >> SD Card Initialization Attempt "); 
    print_dec(i);
    initResp = sd_spiModeInit (&ctv);

    // Failed to initialize if initResp is not 0.
    if (initResp != 0)
    {    
      print_str (": FAILED TO INITIALIZE SD CARD.");
      print_str (" Initialization Error Response: "); 
      sd_printInitError (initResp);
      print_str (", R1 Response: "); 
      sd_printR1 (initResp);
    }
    else
    {   
      print_str (": SD CARD INITIALIZATION SUCCESSFUL");
      break;
    }
  }
  //                                                END SD CARD INITILIAIZATION
  // --------------------------------------------------------------------------



  // initialization successful
  if (initResp == 0)
  {      
    // Variables used for block addressing
    uint32_t startBlckNum;
    uint32_t endBlckNum;
    uint32_t blckNum;
    uint32_t numOfBlcks;

    // To hold the contents of an SD card block
    uint8_t  blckArr[512];

    // 16-bit errors
    uint16_t err16;

    // R2 response. Response to SEND_STATUS.
    uint16_t r2;

    // number of well-written blocks. 
    // Used for multi-block write error.
    uint32_t nwwb;

    // data (string) for single block write.
    uint8_t dataArr2[BLOCK_LEN] = "Would you like to play a game???"; 

    // data (string) written for multi-block write.
    uint8_t dataArr[BLOCK_LEN] = "Well Hi, I See you brought a PIE!!!";

    // for USER INPUT section
    uint8_t answer;
    
/*
    // ----------------------------------------------------------------------
    //                                         READ IN AND PRINT SINGLE BLOCK
    // 
    // Demo sd_readSingleBlock() and sd_printSingleBlock(). This section will
    // use sd_readSingleBlock() to read a block at the address specified into
    // the an array. This array will then be passed to sd_printSingleBlock()
    // which will print the block's contents to the screen.

    // block to read in
    blckNum = 8192;

    // read in a single block located at blckNum
    // on the SD card into the array blckArr.
    if (ctv.type == SDHC)
      err16 = sd_readSingleBlock (blckNum, blckArr);
    else
      err16 = sd_readSingleBlock (blckNum * BLOCK_LEN, blckArr);
    
    if (err16 != READ_SUCCESS)
    { 
      print_str ("\n\r >> sd_readSingleBlock() returned ");
      if (err16 & R1_ERROR)
      {
        print_str ("R1 error: ");
        sd_printR1 (err16);
      }
      else 
      { 
        print_str (" error "); 
        sd_printReadError (err16);
      }
    }

    // print block just loaded into the array.
    else
      sd_printSingleBlock (blckArr);

    //                                       END READ IN AND PRINT SINGLE BLOCK
    // ------------------------------------------------------------------------
*/


/*
    // ------------------------------------------------------------------------
    //                                                    PRINT MULTIPLE BLOCKS
    // 
    // Demo sd_printMultipleBlocks(). This will print multiple, consecutive 
    // SD card blocks beginging at blckNum. The number of blocks printed is
    // specified by numOfBlcks. The function calls the READ_MULTIPLE_BLOCKS 
    // SD card command.

    numOfBlcks = 2;
    blckNum = 20;
    
    if (ctv.type == SDHC) 
      err16 = sd_printMultipleBlocks (blckNum, numOfBlcks);
    else
      err16 = sd_printMultipleBlocks (blckNum * BLOCK_LEN, numOfBlcks);
    
    if (err16 != READ_SUCCESS)
    { 
      print_str ("\n\r >> sd_printMultipleBlocks() returned ");
      if (err16 & R1_ERROR)
      {
        print_str ("R1 error: ");
        sd_printR1 (err16);
      }
      else 
      { 
        print_str (" error "); 
        sd_printReadError  (err16);
      }   
    }
    //                                                END PRINT MULTIPLE BLOCKS
    // ------------------------------------------------------------------------
*/


/*
    // ------------------------------------------------------------------------
    //                                             WRITE and ERASE SINGLE BLOCK
    //
    // Demo sd_writeSingleBlock(), sd_eraseBlocks(), and sd_readSingleBlocks()
    // and sd_printSingleBlocks(). This demo will first erase a single 
    // block using sd_eraseBlocks(), then read in and print the block to 
    // confirm it is erased using sd_readSingleBlock and sd_printSingleBlock().
    // It will then call the sd_writeSingleBlock() function to write data to
    // the block. Finally, it will read in and print the block again to confirm
    // the block was successfully written to. 


    // block to be written.
    blckNum = 20;

    // erase single block (i.e. erase start block = erase end block)
    print_str ("\n\r Erasing Block ");
    print_dec(blckNum);
    if (ctv.type == SDHC)
      err16 = sd_eraseBlocks (blckNum, blckNum);
    else
      err16 = sd_eraseBlocks (blckNum * BLOCK_LEN, blckNum * BLOCK_LEN);
    
    if (err16 != ERASE_SUCCESSFUL)
    {
      print_str ("\n\r >> sd_eraseBlocks() returned ");
      if (err16 & R1_ERROR)
      {
        print_str ("R1 error: ");
        sd_printR1 (err16);
      }
      print_str (" error "); 
      sd_printEraseError (err16);
    }


    // read in and print data block
    print_str ("\n\r Read and Print Block "); 
    print_dec (blckNum); 
    print_str (" after erasing.");
    if (ctv.type == SDHC) 
      err16 = sd_readSingleBlock (blckNum, blckArr);
    else
      err16 = sd_readSingleBlock (blckNum * BLOCK_LEN, blckArr);

    if (err16 != READ_SUCCESS)
    { 
      print_str ("\n\r >> sd_readSingleBlock() returned ");
      if (err16 & R1_ERROR)
      {
        print_str ("R1 error: ");
        sd_printR1 (err16);
      }
      else 
      { 
        print_str (" error "); 
        sd_printReadError (err16);
      }
    }
    // print data block just loaded into the array.
    else
      sd_printSingleBlock (blckArr);

    // write to data block
    print_str("\n\r Write to block "); 
    print_dec(blckNum);
    if (ctv.type == SDHC) 
      err16 = sd_writeSingleBlock (blckNum, dataArr);
    else
      err16 = sd_writeSingleBlock (blckNum * BLOCK_LEN, dataArr);

    if (err16 != DATA_ACCEPTED_TOKEN_RECEIVED)
    { 
      print_str ("\n\r >> sd_writeSingleBlock() returned ");
      if (err16 & R1_ERROR)
      {
        print_str ("R1 error: ");
        sd_printR1 (err16);
      }
      else 
      { 
        print_str ("error "); 
        sd_printWriteError (err16);

        // Get the R2 (SEND_STATUS) response if the Write Error Token
        // was returned by the card while writing to the block.
        // May convert this to a function. 
        if ((err16 & WRITE_ERROR_TOKEN_RECEIVED) == WRITE_ERROR_TOKEN_RECEIVED)
        {
          print_str ("\n\r WRITE ERROR TOKEN returned.");
          print_str ("Getting R2 response.");
          
          CS_SD_LOW;             
          sd_sendCommand (SEND_STATUS,0);
          // The first byte of R2 is R1
          r2 = sd_getR1(); 
          r2 <<= 8;
          r2 |= sd_receiveByteSPI();
          CS_SD_HIGH;
          print_str ("\n\r R2 Response = ");
          print_dec (r2);
        }
      }
    }

    // Verify data has been written to the block 
    // by reading in and printing the block
    else 
    {
      print_str("\n\r Read and Print Block "); 
      print_dec(blckNum); 
      print_str(" after writing");

      if (ctv.type == SDHC)
        err16 = sd_readSingleBlock (blckNum, blckArr);
      else
        err16 = sd_readSingleBlock (blckNum * BLOCK_LEN, blckArr);

      if (err16 != READ_SUCCESS)
      { 
        print_str ("\n\r >> sd_readSingleBlock() returned ");
        if (err16 & R1_ERROR)
        {
          print_str ("R1 error: ");
          sd_printR1 (err16);
        }
        else 
        { 
          print_str (" error "); 
          sd_printReadError  (err16);
        }
      }
      // print the single data block that was just read in.
      else
        sd_printSingleBlock (blckArr);
    }
    //                                         END WRITE and ERASE SINGLE BLOCK
    // ------------------------------------------------------------------------
*/


/*
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


    startBlckNum = 20;
    endBlckNum   = 22;                                     // used for erasing
    numOfBlcks   = 2;                                      // used for writing

    // erase multiple blocks
    print_str("\n\r ERASING BLOCKS ");
    if (ctv.type == SDHC)
      err16 = sd_eraseBlocks (startBlckNum, endBlckNum);
    else // SDSC
      err16 = sd_eraseBlocks (startBlckNum * BLOCK_LEN, 
                              endBlckNum * BLOCK_LEN);
    
    if(err16 != ERASE_SUCCESSFUL)
    {
      print_str ("\n\r >> sd_eraseBlocks() returned ");
      if (err16 & R1_ERROR)
      {
        print_str ("R1 error: ");
        sd_printR1 (err16);
      }
      print_str ("error "); 
      sd_printEraseError (err16);
    }


    // Print Multiple Blocks
    print_str ("\n\r PRINTING BLOCKS BEFORE WRITE ");
    if (ctv.type == SDHC)
      err16 = sd_printMultipleBlocks (startBlckNum, numOfBlcks);
    else 
      err16 = sd_printMultipleBlocks (startBlckNum * BLOCK_LEN, numOfBlcks);
    
    if (err16 != READ_SUCCESS)
    { 
      print_str ("\n\r >> sd_printMultipleBlocks() returned ");
      if (err16 & R1_ERROR)
      {
        print_str ("R1 error: ");
        sd_printR1 (err16);
      }
      else 
      { 
        print_str (" error "); 
        sd_printReadError (err16);
      }
    }

    // Write Multiple Blocks
    print_str ("\n\r WRITING BLOCKS ");
    err16 = sd_writeMultipleBlocks (startBlckNum, numOfBlcks, dataArr2);
    if (err16 != DATA_ACCEPTED_TOKEN_RECEIVED)
    { 
      print_str ("\n\r >> sd_writeMultipleBlocks() returned ");
      if (err16 & R1_ERROR)
      {
        print_str ("R1 error: ");
        sd_printR1 (err16);
      }
      else 
      { 
        print_str ("error "); 
        sd_printWriteError (err16);

        // Get the R2 (SEND_STATUS) response if the Write Error Token
        // was returned by the card while writing to the block.
        if ((err16 & WRITE_ERROR_TOKEN_RECEIVED) == WRITE_ERROR_TOKEN_RECEIVED)
        {
          // Getting R2.  May make this a function.
          print_str ("\n\r WRITE_ERROR_TOKEN set.");
          print_str ("\n\r Getting STATUS (R2) response.");
          CS_SD_LOW;             
          sd_sendCommand (SEND_STATUS, 0);
          // The first byte of R2 is R1
          r2 = sd_getR1(); 
          r2 <<= 8;
          r2 |= sd_receiveByteSPI();
          CS_SD_HIGH;
          print_str ("\n\r R2 Response = ");
          print_dec (r2);

          // Number of Well Written Blocks
          print_str ("\n\r Getting Number of Well Written Blocks.");
          
          err16 = sd_getNumOfWellWrittenBlocks (&nwwb);
          if (err16 != READ_SUCCESS)
          { 
            print_str("\n\r >> SD_GetNumberOfWellWritteBlocks() returned ");
            if (err16 & R1_ERROR)
            {
              print_str("R1 error: ");
              sd_printR1(err16);
            }
            else 
            { 
              print_str("error "); 
              sd_printReadError (err16);
            }
            print_str("\n\r Number of well written write blocks = ");
            print_dec(nwwb);
          }
        }
      }
    }
    
    
    // post multi-block write READ amd PRINT blocks.
    print_str("\n\r PRINTING BLOCKS AFTER WRITE ");
    if (ctv.type == SDHC)
      err16 = sd_printMultipleBlocks (startBlckNum, numOfBlcks);
    else
      err16 = sd_printMultipleBlocks (startBlckNum * BLOCK_LEN, numOfBlcks);
    
    if (err16 != READ_SUCCESS)
    { 
      print_str ("\n\r >> sd_printMultipleBlocks() returned ");
      if (err16 & R1_ERROR)
      {
        print_str ("R1 error: ");
        sd_printR1 (err16);
      }
      else 
      { 
        print_str ("error "); 
        sd_printReadError  (err16);
      }
    }
    //                                      END WRITE and ERASE MULTIPLE BLOCKS
    // ------------------------------------------------------------------------
*/


/*
    // ------------------------------------------------------------------------
    //                                                       USER INPUT SECTION
    //
    // This section allows a user to interact with the function 
    // sd_printMultipleBlocks(). The user is asked which block number they 
    // would like to print first, and then how many blocks they would like to 
    // print. The sd_printMultipleBlocks() function is then called with these
    // parameters and the blocks specified by the user are printed.

    do
    {
      do
      {
        print_str ("\n\n\n\rEnter Start Block\n\r");
        startBlckNum = enterBlockNumber();
        print_str ("\n\rHow many blocks do you want to print?\n\r");
        numOfBlcks = enterBlockNumber();
        print_str ("\n\rYou have selected to print "); 
        print_dec(numOfBlcks);
        print_str (" blocks beginning at block number "); 
        print_dec(startBlckNum);
        print_str ("\n\rIs this correct? (y/n)");
        answer = usart_receive();
        usart_transmit (answer);
        print_str ("\n\r");
      }
      while (answer != 'y');

      // print the blocks specified above.
      if (ctv.type == SDHC) 
        err16 = sd_printMultipleBlocks (startBlckNum, numOfBlcks);
      else
        err16 = sd_printMultipleBlocks (startBlckNum * BLOCK_LEN, 
                                        numOfBlcks);
      
      if (err16 != READ_SUCCESS)
      { 
        print_str ("\n\r >> sd_printMultipleBlocks() returned ");
        if (err16 & R1_ERROR)
        {
          print_str ("R1 error: ");
          sd_printR1 (err16);
        }
        else 
        { 
          print_str (" error "); 
          sd_printReadError  (err16);
        }
      }

      print_str ("\n\rPress 'q' to quit: ");
      answer = usart_receive();
      usart_transmit (answer);
    }
    while (answer != 'q');

    //                                                   END USER INPUT SECTION
    // ------------------------------------------------------------------------
*/


/*
    // ------------------------------------------------------------------------
    //                                                          MEMORY CAPACITY
    //
    // Get the total memory capacity of the card, in bytes. How this is
    // calculated depends on the card type (i.e. ctv.type) which is used to 
    // select the correct function.

    print_str ("\n\n\n\r Memory capacity = ");
    if (ctv.type == SDHC) 
      print_dec (sd_getMemoryCapacitySDHC ());
    else // SDSC
      print_dec (sd_getMemoryCapacitySDSC ());
    print_str( " Bytes");
    //                                                      END MEMORY CAPACITY
    // ------------------------------------------------------------------------
*/


/*
    // ------------------------------------------------------------------------
    //                                                FIND NON-ZERO DATA BLOCKS
    //
    // Uses the function sd_findNonZeroDataBlockNums() to print the block 
    // numbers of any that have non-zero data. I made this to help locate raw 
    // data on the SD card. This is function is not fast in printing to the 
    // screen, so I do not suggest using it over a large range of blocks.

    print_str ("\n\n\r\r sd_findNonZeroDataBlockNums() \n\r");
    startBlckNum = 8000;
    endBlckNum = 9000;

    if (ctv.type == SDHC) 
      sd_findNonZeroDataBlockNums (startBlckNum, endBlckNum);
    else
      sd_findNonZeroDataBlockNums (startBlckNum * BLOCK_LEN, 
                                    endBlckNum * BLOCK_LEN);
    print_str("\n\r Done\n\r");
    //                                                      END MEMORY CAPACITY
    // ------------------------------------------------------------------------
*/
  }
  // End SD Card Testing


  // This is just something to do after SD card testing has completed.
  while (1)
    usart_transmit(usart_receive());
  
  
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

  c = usart_receive();
  
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
      print_str ("\b ");
      blckNum = blckNum / 10;
    }

    print_str ("\r");
    print_dec (blckNum);
    
    if (blckNum >= 4194304)
    {
      blckNum = 0;
      print_str ("\n\rblock number is too large.");
      print_str ("\n\rEnter value < 4194304\n\r");
    }
    c = usart_receive();
  }
  return blckNum;
}