/*
*******************************************************************************
*                                AVR-SD CARD MODULE
*
* File    : SD_TEST.C
* Author  : Joshua Fain
* Target  : ATMega1280
* License : MIT
* Copyright (c) 2020
*
*
* DESCRIPTION:
* Examples to implement and test various capabilities of the AVR-SD Card 
* Module. Contains main().
*******************************************************************************
*/

#include <stdint.h>
#include <avr/io.h>
#include "../includes/usart.h"
#include "../includes/spi.h"
#include "../includes/prints.h"
#include "../includes/sd_spi_base.h"
#include "../includes/sd_spi_data_access.h"
#include "../includes/sd_spi_misc.h"


// local function
uint32_t enterBlockNumber();


int main(void)
{
  usart_init();
  spi_masterInit();


  // ************** SD CARD INITILIAIZATION 
  //
  // Set ctv and initialize the SD card using the initialization 
  // routine. An instance of CTV is required to be set in order
  // to determine how to address the blocks on the SD card.

  CTV ctv;

  uint32_t initResp;

  // Attempt, up to 5 times, to initialize the SD card.
  for (uint8_t i = 0; i < 5; i++)
    {
      print_str ("\n\n\r >> SD Card Initialization Attempt "); 
      print_dec(i);
      initResp = sd_spiModeInit (&ctv);

      // If R1 and Initialization Error Flag portions of the response
      // are both 0 then the initialization was successful.
      if (((initResp & 0xFF) != 0) && ((initResp & 0xFFF00) != 0))
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


  // initialization successful
  if (initResp == 0)
    {      
      // Some variables used for block addressing
      // in the subsequent functions
      uint32_t startBlckNum;
      uint32_t endBlckNum;
      uint32_t blckNum;
      uint32_t numOfBlcks;

      // Array to hold the contents of an SD card block
      uint8_t  blckArr[512];

      // 16-bit errors
      uint16_t err16; 
      

      // ********** TEST: sd_readSingleBlock() 
      // 
      // Read in a single block from the SD card at location specified by
      // blckNum into the array, blckArr. This also demonstrates how
      // to use sd_printSingleBlock() to print the block.

      blckNum = 8192; // random block number

      // read in a single block of BLOCK_LEN 
      // located at blckNum on the SD card.

      if (ctv.type == SDHC) // SDHC? card is block addressable
        err16 = sd_readSingleBlock (blckNum, blckArr);
      else // SDSC is byte addressable
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

      // print single data block just loaded into the array.
      else
        sd_printSingleBlock (blckArr);

      // **********


/*
      // ********** TEST: sd_printMultipleBlocks()
      //
      // Print multiple, consecutive SD card blocks begginging at blckNum.
      // The number of blocks printed is specified by numOfBlcks. The 
      // function calls the READ_MULTIPLE_BLOCKS SD card command.
      
      numOfBlcks = 2;
      
      blckNum = 20;
      
      if (ctv.type == SDHC) 
        err16 = sd_printMultipleBlocks (blckNum,numOfBlcks);
      else  // SDSC
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
      // **********



      // ********** TEST: sd_writeSingleBlock()
      //
      // Write data to a single block on the SD card specified by blckNum.
      // This demo will first erase a single block using sd_eraseBlocks(),
      // read and print the block to confirm it is erased using
      // sd_readSingleBlock and sd_printSingleBlock(), then call the 
      // sd_writeSingleBlock() function, and read and print the block 
      // again to confirm the write was successful.


      // data to write to block
      uint8_t dataArr[BLOCK_LEN] = "Howdy Globe......"; 
  
      blckNum = 20;

      // ERASE single block (erase START block = erase END block)
      print_str("\n\r Erasing Block "); print_dec(blckNum);
      if (ctv.type == SDHC)
        err16 = sd_eraseBlocks (blckNum, blckNum);
      else // SDSC
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


      // READ and PRINT
      print_str("\n\r Read and Print Block "); 
      print_dec(blckNum); 
      print_str(" after erasing.");
      if (ctv.type == SDHC) 
        err16 = sd_readSingleBlock (blckNum, blckArr);
      else //SDSC
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
      // print the data block just loaded into the array.
      else
        sd_printSingleBlock (blckArr);

      // WRITE
      print_str("\n\r Write to block "); print_dec(blckNum);
      err16 = sd_writeSingleBlock (blckNum, dataArr);
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
              print_str (" error "); 
              sd_printWriteError (err16);

              // Get the R2 (SEND_STATUS) response if the Write Error Token
              // was returned by the card while writing to the block.
              // May convert this to a function. 
              if ((err16 & WRITE_ERROR_TOKEN_RECEIVED) 
                   == WRITE_ERROR_TOKEN_RECEIVED)
                {
                  print_str ("\n\r WRITE ERROR TOKEN returned.");
                  print_str ("Getting R2 response.");
                  
                  uint16_t r2;
                  CS_SD_LOW;             
                  sd_sendCommand (SEND_STATUS,0);
                  r2 = sd_getR1(); // The first byte of R2 is R1
                  r2 <<= 8;
                  r2 |= sd_receiveByteSPI();
                  CS_SD_HIGH;
                  print_str ("\n\r R2 Response = ");
                  print_dec (r2);
                }
            }
        }

      // READ amd PRINT
      else // Verify data has been written to block
        {
          print_str("\n\r Read and Print Block "); 
          print_dec(blckNum); 
          print_str(" after writing");

          if (ctv.type == SDHC)
            err16 = sd_readSingleBlock (blckNum, blckArr);
          else // SDSC
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
          else// print the single data block that was read just in.
            sd_printSingleBlock (blckArr);
        }
      // **********




      // ********** TEST: sd_writeMultipleBlocks()
      //
      // Write to multiple blocks of the SD card. This function will write the
      // same data to the blocks specified by startBlckNum and numOfBlcks. This
      // function is not really useful as it writes the same data to every 
      // block, but is used to demonstrate the WRITE_MULTIPLE_BLOCK command. 
      // This block of code will first erase the multiple blocks using 
      // sd_eraseBlocks(), then read-in and print the same blocks using 
      // sd_printMultipleBlocks(). Then it will write to the multiple blocks. 
      // It should then do some error handling if WRITE_BLOCK_TOKEN_RECIEVED 
      // is returned, but I haven't really been able to test this out. It will
      // then read-in and print the blocks again to show successful write. 


      // data to be written to the blocks
      uint8_t dataArr2[BLOCK_LEN] 
              = "Well Hello There! I see you brought a PIE!!";

      startBlckNum = 20;
      endBlckNum   = 22; // used for erasing
      numOfBlcks   = 2;  // used for writing

      // ERASE multiple blocks
      print_str("\n\r ***** ERASING BLOCKS ***** ");
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
              print_str("R1 error: ");
              sd_printR1(err16);
            }
          print_str(" error "); 
          sd_printEraseError (err16);
        }

      // Print Multiple Blocks
      print_str ("\n\r ***** PRINTING BLOCKS BEFORE WRITE ***** ");
      if (ctv.type == SDHC)
        err16 = sd_printMultipleBlocks (startBlckNum, numOfBlcks);
      else // SDSC
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
              sd_printReadError  (err16);
            }
        }

      // Write Multiple Blocks
      print_str ("\n\r ***** WRITING BLOCKS ***** ");
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
              print_str (" error "); 
              sd_printWriteError (err16);

              // Get the R2 (SEND_STATUS) response if the Write Error Token
              // was returned by the card while writing to the block.
              // May convert this to a function. 
              if (( err16 & WRITE_ERROR_TOKEN_RECEIVED) == WRITE_ERROR_TOKEN_RECEIVED)
                {
                  // Getting R2.  May make this into a function.
                  print_str ("\n\r WRITE_ERROR_TOKEN set.");
                  print_str ("\n\r Getting STATUS (R2) response.");
                  uint16_t r2;
                  CS_SD_LOW;             
                  sd_sendCommand (SEND_STATUS,0);
                  r2 = sd_getR1(); // The first byte of R2 is R1
                  r2 <<= 8;
                  r2 |= sd_receiveByteSPI();
                  CS_SD_HIGH;
                  print_str ("\n\r R2 Response = ");
                  print_dec (r2);

                  // Number of Well Written Blocks
                  print_str ("\n\r Getting Number of Well Written Blocks.");
                  uint32_t nwwb;
                  err16 = sd_getNumOfWellWrittenBlocks (&nwwb);
                  if (err16 != READ_SUCCESS)
                    { 
                      print_str("\n\r >> SD_GetNumberOfWellWritteBlocks() returned ");
                      if(err16 & R1_ERROR)
                        {
                          print_str("R1 error: ");
                          sd_printR1(err16);
                        }
                      else 
                        { 
                          print_str(" error "); 
                          sd_printReadError (err16);
                        }
                      print_str("\n\r Number of well written write blocks = ");
                      print_dec(nwwb);
                    }
                }
            }
        }


      print_str("\n\r ***** PRINTING BLOCKS AFTER WRITE ***** ");
      // READ amd PRINT post write

      if (ctv.type == SDHC)
        err16 = sd_printMultipleBlocks (startBlckNum, numOfBlcks);
      else // SDSC
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
              sd_printReadError  (err16);
            }
        }
      // **********



      // ********** User Input Section
      //
      // This section allows a user to interact with the the function 
      // sd_printultipleBlocks() by entering the block number to begin the 
      // multile block read at and then setting the number of blocks to read.

      uint8_t  answer;
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

          // READ amd PRINT post write

          // SDHC is block addressable
          if (ctv.type == SDHC) 
            err16 = sd_printMultipleBlocks (startBlckNum,numOfBlcks);
          // SDSC is byte addressable
          else // (SDSC)
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
      // **********

*/

      // ********************************************************************
      //                          SD_SPI_MISC FUNCTIONS
      // ********************************************************************


      // ********** SD_GetMemoryCapacityHC/SC
      //
      // Get SD Card's memory capacity and print it
      // to the screen. Still testing these....

      print_str ("\n\n\n\r Memory capacity = ");
      if (ctv.type == SDHC) 
        print_dec (sd_getMemoryCapacitySDHC ());
      else // SDSC
        print_dec (sd_getMemoryCapacitySDSC ());
      print_str( " Bytes");
      // **********
      


      // ********** SD_Fsd_findNonZeroDataBlockNums ()
      //
      // print the block numbers of the blocks that have non-zero data.
      // I made this to help locate raw data on the SD card. This is not
      // fast, so don't use it over a large range of blocks.

      print_str ("\n\n\r\r SD_Fsd_findNonZeroDataBlockNums () \n\r");
      startBlckNum = 5;
      endBlckNum = 30;

      // SDHC is block addressable
      if (ctv.type == SDHC) 
        sd_findNonZeroDataBlockNums (startBlckNum, endBlckNum);
      // SDSC is block addressable
      else // SDSC
        sd_findNonZeroDataBlockNums (startBlckNum * BLOCK_LEN, 
                                     endBlckNum * BLOCK_LEN);
      print_str("\n\r Done\n\r");
      // **********
  }



  // Something to do after SD card testing has completed.
  while(1)
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
      if ((c >= '0') && (c <= '9'))
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
          print_str("\n\rblock number is too large.");
          print_str("\n\rEnter value < 4194304\n\r");
        }
      c = usart_receive();
    }
  return blckNum;
}
