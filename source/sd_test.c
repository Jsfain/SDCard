/*
***********************************************************************************************************************
*                                                   AVR-SDCARD MODULE
*
* File    : SD_TEST.C
* Version : ?
* Author  : Joshua Fain
* Target  : ATMega1280
*
*
* DESCRIPTION:
* Contains main and includes some examples to test the AVR-SDCard Module
* 
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
#include "../includes/sd_spi_misc.h"


// local function
uint32_t enterBlockNumber();


int main(void)
{
    USART_Init();
    SPI_MasterInit();


    // ******************* SD CARD INITILIAIZATION ****************
    //
    // Initializing ctv. These members will be set by the SD card's
    // initialization routine. They should only be set there.
    
    CTV ctv;

    uint32_t initResponse;

    // Attempt, up to 10 times, to initialize the SD card.
    for(int i = 0; i < 10; i++)
    {
        print_str("\n\n\r SD Card Initialization Attempt # "); print_dec(i);
        initResponse = SD_InitializeSPImode(&ctv);
        if( ( (initResponse & 0xFF) != OUT_OF_IDLE) && 
            ( (initResponse & 0xFFF00) != INIT_SUCCESS ) )
        {    
            print_str("\n\n\r FAILED INITIALIZING SD CARD");
            print_str("\n\r Initialization Error Response: "); 
            SD_PrintInitError(initResponse);
            print_str("\n\r R1 Response: "); SD_PrintR1(initResponse);
        }
        else
        {   print_str("\n\r SUCCESSFULLY INITIALIZED SD CARD");
            break;
        }
    }

    if(initResponse == OUT_OF_IDLE) // initialization successful
    {      
        // create some variables to use for addressing
        // in the subsequent functions
        uint32_t startBlockNumber;
        uint32_t endBlockNumber;
        uint32_t blockNumber;
        uint32_t numberOfBlocks;

        uint8_t ds[512];
        uint16_t err16; // returned error values
        
/*
        // ******************* SD_ReadSingleBlock() ************************
        // 
        // Read in a single block from the SD card specified by blockNumber 
        // into the array member of ds. This also demonstrates how to use 
        // SD_PrintSingleBlock() to print the block just read into the array.

        blockNumber = 8192;
        if (ctv.type == SDHC) // SDHC is block addressable
            err16 = SD_ReadSingleBlock(blockNumber, ds);
        else // SDSC byte addressable
            err16 = SD_ReadSingleBlock(blockNumber * BLOCK_LEN, ds);
        
        if(err16 != READ_SUCCESS)
        { 
            print_str("\n\r >> SD_ReadSingleBlock() returned ");
            if(err16 & R1_ERROR)
            {
                print_str("R1 error: ");
                SD_PrintR1(err16);
            }
            else 
            { 
                print_str(" error "); 
                SD_PrintReadError(err16);
            }
        }
        
        else// print the single data block that was read just in.
            SD_PrintSingleBlock(ds);

        // **********************************************************
*/     


/*
        // ******************* SD_PrintMultipleBlocks() **********************
        //
        // Use this function to print multiple SD card blocks back-to-back
        // beginning at blockNumber. The number of blocks printed is specified
        // numberOfBlocks. 
        // This function calls the READ_MULTIPLE_BLOCKS SD card command.
        
        numberOfBlocks = 3;
        
        blockNumber = 8192;
          
        if (ctv.type == SDHC) // SDHC is block addressable
            err16 = SD_PrintMultipleBlocks(blockNumber,numberOfBlocks);
        else // SDSC byte addressable
            err16 = SD_PrintMultipleBlocks(
                            blockNumber * BLOCK_LEN, 
                            numberOfBlocks);
        
        if(err16 != READ_SUCCESS)
        { 
            print_str("\n\r >> SD_PrintMultipleBlock() returned ");
            if(err16 & R1_ERROR)
            {
                print_str("R1 error: ");
                SD_PrintR1(err16);
            }
            else 
            { 
                print_str(" error "); 
                SD_PrintReadError(err16);
            }
        }
        // *******************************************************************
*/


/*
        // ******************* SD_WriteSingleBlock() **************************
        //
        // Use this function to write to a single block on the SD card 
        // specified by blockNumber. This demo will first erase a single block
        // using SD_EraseBlocks(), read and print the same block using
        // SD_ReadSingleBlock and SD_PrintSingleBlock() to show successful erase. It
        // will writes to the block. Finally, it reads in and prints the block
        // again to demonstrate the write was succesful.
        

        // data to write to block
        uint8_t db[BLOCK_LEN] = "HELLO WORLD......"; 
    
        blockNumber = 10;

        // ERASE single block (start and end block is the same)
        if (ctv.type == SDHC) // SDHC is block addressable
            err16 = SD_EraseBlocks(blockNumber, blockNumber);
        else // SDSC byte addressable
            err16 = SD_EraseBlocks(
                    blockNumber * BLOCK_LEN,
                    blockNumber * BLOCK_LEN);
        
        if(err16 != ERASE_SUCCESSFUL)
        {
            print_str("\n\r >> SD_EraseBlocks() returned ");
            if(err16 & R1_ERROR)
            {
                print_str("R1 error: ");
                SD_PrintR1(err16);
            }

            print_str(" error "); 
            SD_PrintEraseError(err16);
        }


        // READ and PRINT
        if (ctv.type == SDHC) // SDHC is block addressable
            err16 = SD_ReadSingleBlock(blockNumber, ds);
        else // SDSC byte addressable
            err16 = SD_ReadSingleBlock(blockNumber * BLOCK_LEN, ds);

        if(err16 != READ_SUCCESS)
        { 
            print_str("\n\r >> SD_ReadSingleBlock() returned ");
            if(err16 & R1_ERROR)
            {
                print_str("R1 error: ");
                SD_PrintR1(err16);
            }
            else 
            { 
                print_str(" error "); 
                SD_PrintReadError(err16);
            }
        }
        else// print the single data block that was read just in.
            SD_PrintSingleBlock(ds);

        // WRITE
        err16 = SD_WriteSingleBlock(blockNumber,db);
        if(err16 != DATA_ACCEPTED_TOKEN)
        { 
            print_str("\n\r >> SD_WriteSingleBlock() returned ");
            if(err16 & R1_ERROR)
            {
                print_str("R1 error: ");
                SD_PrintR1(err16);
            }
            else 
            { 
                print_str(" error "); 
                SD_PrintWriteError(err16);

                // good to get the R2 (SEND_STATUS) response if the 
                // WRITE_ERROR_TOKEN was returned by the card while writing to
                // the block. May make this into a function. 
                if (( err16 & WRITE_ERROR_TOKEN) == WRITE_ERROR_TOKEN)
                {
                    print_str("\n\r WRITE_ERROR_TOKEN set. Getting R2 response.");
                    
                    uint16_t r2;
                    CS_LOW;             
                    SD_SendCommand(SEND_STATUS,0);
                    r2 = SD_GetR1(); // The first byte of R2 is R1
                    r2 <<= 8;
                    r2 |= SD_ReceiveByteSPI();
                    CS_HIGH;
                    print_str("\n\r R2 Response = ");
                    print_dec(r2);
                }
            }
        }

        // READ amd PRINT
        else // Verify data has been written to block
        {
            if (ctv.type == SDHC) // SDHC is block addressable
                err16 = SD_ReadSingleBlock(blockNumber, ds);
            else // SDSC byte addressable
                err16 = SD_ReadSingleBlock(blockNumber * BLOCK_LEN, ds);

            if(err16 != READ_SUCCESS)
            { 
                print_str("\n\r >> SD_ReadSingleBlock() returned ");
                if(err16 & R1_ERROR)
                {
                    print_str("R1 error: ");
                    SD_PrintR1(err16);
                }
                else 
                { 
                    print_str(" error "); 
                    SD_PrintReadError(err16);
                }
            }
            else// print the single data block that was read just in.
                SD_PrintSingleBlock(ds);
        }
        // *******************************************************************
*/


/*
        // ******************* SD_WriteMultipleBlocks() ***********************
        //
        // Use this function to write to multiple blocks of the SD card. This
        // function will write the same data to the blocks specified by 
        // startBlockNumber and numberOfBlocks. This function is not really
        // useful as it writes the same data to every block, but is used to 
        // demonstrate the WRITE_MULTIPLE_BLOCK command. This block of code
        // will first erase the multiple blocks using SD_EraseBlocks(), then
        // read-in and print the same blocks using SD_PrintMultipleBlocks().
        // Then it will write to the multiple blocks, it will then do some
        // error handling if the WRITE_BLOCK_TOKEN is received (This should
        // work but I haven't really been able to test this out though). It
        // will then read-in and print the blocks again to show that the data 
        // was successfully written to the blocks.
        

        // data to write to block
        uint8_t mdb[BLOCK_LEN] = "Well Hello There! I see you brought a PIE!!";

        startBlockNumber = 10;
        endBlockNumber = 20;
        numberOfBlocks = 2;

        // ERASE multiple blocks
        print_str("\n\r ***** ERASING BLOCKS ***** ");
        if (ctv.type == SDHC) // SDHC is block addressable
            err16 = SD_EraseBlocks(startBlockNumber, endBlockNumber);
        else // SDSC byte addressable
            err16 = SD_EraseBlocks(
                    startBlockNumber * BLOCK_LEN,
                    endBlockNumber * BLOCK_LEN);
        
        if(err16 != ERASE_SUCCESSFUL)
        {
            print_str("\n\r >> SD_EraseBlocks() returned ");
            if(err16 & R1_ERROR)
            {
                print_str("R1 error: ");
                SD_PrintR1(err16);
            }

            print_str(" error "); 
            SD_PrintEraseError(err16);
        }

        // PRINT Multiple Blocks
        print_str("\n\r ***** PRINTING BLOCKS BEFORE WRITE ***** ");
        if (ctv.type == SDHC) // SDHC is block addressable
            err16 = SD_PrintMultipleBlocks(startBlockNumber,numberOfBlocks);
        else // SDSC byte addressable
            err16 = SD_PrintMultipleBlocks(
                            startBlockNumber * BLOCK_LEN, 
                            numberOfBlocks);
        
        if(err16 != READ_SUCCESS)
        { 
            print_str("\n\r >> SD_PrintMultipleBlocks() returned ");
            if(err16 & R1_ERROR)
            {
                print_str("R1 error: ");
                SD_PrintR1(err16);
            }
            else 
            { 
                print_str(" error "); 
                SD_PrintReadError(err16);
            }
        }

        // WRITE Multiple Blocks
        print_str("\n\r ***** WRITING BLOCKS ***** ");
        err16 = SD_WriteMultipleBlocks(startBlockNumber,numberOfBlocks,mdb);
        if(err16 != DATA_ACCEPTED_TOKEN)
        { 
            print_str("\n\r >> SD_WriteMultipleBlocks() returned ");
            if(err16 & R1_ERROR)
            {
                print_str("R1 error: ");
                SD_PrintR1(err16);
            }
            else 
            { 
                print_str(" error "); 
                SD_PrintWriteError(err16);

                // should get R2 (SEND_STATUS) and get the number of well
                // written blocks if the WRITE_ERROR_TOKEN was returned
                // by the card while writing attempting to write.
                 
                if (( err16 & WRITE_ERROR_TOKEN) == WRITE_ERROR_TOKEN)
                {
                    // Getting R2.  May make this into a function.
                    print_str("\n\r WRITE_ERROR_TOKEN set.");
                    print_str("\n\r Getting STATUS (R2) response.");
                    uint16_t r2;
                    CS_LOW;             
                    SD_SendCommand(SEND_STATUS,0);
                    r2 = SD_GetR1(); // The first byte of R2 is R1
                    r2 <<= 8;
                    r2 |= SD_ReceiveByteSPI();
                    CS_HIGH;
                    print_str("\n\r R2 Response = ");
                    print_dec(r2);

                    // Number of Well Written Blocks
                    print_str("\n\r Getting Number of Well Written Blocks.");
                    uint32_t nwwb;
                    err16 = SD_GetNumberOfWellWrittenBlocks(&nwwb);
                    if(err16 != READ_SUCCESS)
                    { 
                        print_str("\n\r >> SD_GetNumberOfWellWritteBlocks() returned ");
                        if(err16 & R1_ERROR)
                        {
                            print_str("R1 error: ");
                            SD_PrintR1(err16);
                        }
                        else 
                        { 
                            print_str(" error "); 
                            SD_PrintReadError(err16);
                        }
                        print_str("\n\r Number of well written write blocks = ");
                        print_dec(nwwb);
                    }
                }
            }
        }


        print_str("\n\r ***** PRINTING BLOCKS AFTER WRITE ***** ");
        // READ amd PRINT post write
        if (ctv.type == SDHC) // SDHC is block addressable
            err16 = SD_PrintMultipleBlocks(startBlockNumber,numberOfBlocks);
        else // SDSC byte addressable
            err16 = SD_PrintMultipleBlocks(
                            startBlockNumber * BLOCK_LEN, 
                            numberOfBlocks);
        
        if(err16 != READ_SUCCESS)
        { 
            print_str("\n\r >> SD_PrintMultipleBlocks() returned ");
            if(err16 & R1_ERROR)
            {
                print_str("R1 error: ");
                SD_PrintR1(err16);
            }
            else 
            { 
                print_str(" error "); 
                SD_PrintReadError(err16);
            }
        }
        // ********************************************************************
*/


        // ************************ User Input Section ************************
        //
        // This section allows a user to interact with the the function 
        // SD_ReadMultipleBlocks() by entering the block number to begin the 
        // multile block read at and then setting the number of blocks to read.

        uint8_t  answer;
        do{
            do{
                print_str("\n\n\n\rEnter Start Block\n\r");
                startBlockNumber = enterBlockNumber();
                print_str("\n\rHow many blocks do you want to print?\n\r");
                numberOfBlocks = enterBlockNumber();
                print_str("\n\rYou have selected to print "); print_dec(numberOfBlocks);
                print_str(" blocks beginning at block number "); print_dec(startBlockNumber);
                print_str("\n\rIs this correct? (y/n)");
                answer = USART_Receive();
                USART_Transmit(answer);
                print_str("\n\r");
            }while(answer != 'y');

            // READ amd PRINT post write
            if (ctv.type == SDHC) // SDHC is block addressable
                err16 = SD_PrintMultipleBlocks(startBlockNumber,numberOfBlocks);
            else // SDSC byte addressable
                err16 = SD_PrintMultipleBlocks(
                                startBlockNumber * BLOCK_LEN, 
                                numberOfBlocks);
            
            if(err16 != READ_SUCCESS)
            { 
                print_str("\n\r >> SD_PrintMultipleBlocks() returned ");
                if(err16 & R1_ERROR)
                {
                    print_str("R1 error: ");
                    SD_PrintR1(err16);
                }
                else 
                { 
                    print_str(" error "); 
                    SD_PrintReadError(err16);
                }
            }

            print_str("\n\rPress 'q' to quit: ");
            answer = USART_Receive();
            USART_Transmit(answer);
        }while(answer != 'q');
        // ********************************************************************


        // ********************************************************************
        //                          SD_SPI_MISC FUNCTIONS
        // ********************************************************************


        // *******  SD_GetMemoryCapacityHC/SC *******
        //
        // Get SD Card's memory capacity and print it
        // to the screen. Still testing these....
        print_str("\n\n\n\r Memory capacity = ");
        if (ctv.type == SDHC) 
            print_dec(SD_GetMemoryCapacityHC());
        else  
            print_dec(SD_GetMemoryCapacitySC());
        print_str(" Bytes");
        // ******************************************
        


        // ***************** SD_FindNonZeroBlockNumbers() ******************
        //
        // print the block numbers of those block entries that have non-zero
        // entries. I made this to help locate raw data on the SD card.
        // This is not fast, so don't use it over a large range of blocks.
        print_str("\n\n\r\r ***** SD_FindNonZeroBlockNumbers() ******\n\r");
        startBlockNumber = 5;
        endBlockNumber = 30;

        if (ctv.type == SDHC) // SDHC is block addressable
            SD_FindNonZeroBlockNumbers(startBlockNumber, endBlockNumber);
        else // SDSC byte addressable
            SD_FindNonZeroBlockNumbers( 
                        startBlockNumber * BLOCK_LEN, 
                        endBlockNumber * BLOCK_LEN);
        print_str("\n\r Done\n\r");
        // *****************************************************************
    }



    // Something to do after SD card testing has completed.
    while(1)
        USART_Transmit(USART_Receive());
    return 0;
}





// local function for taking user input to specify a block
// number. If nothing is entered then the block number is 0.
uint32_t enterBlockNumber()
{
    uint8_t x;
    uint8_t c;
    uint32_t blockNumber = 0;

    c = USART_Receive();
    
    while(c!='\r')
    {
        if( (c >= '0') && (c <= '9') )
        {
            x = c - '0';
            blockNumber = blockNumber * 10;
            blockNumber += x;
        }
        else if ( c == 127) // backspace
        {
            print_str("\b ");
            blockNumber = blockNumber/10;
        }

        print_str("\r");
        print_dec(blockNumber);
        
        if(blockNumber >= 4194304)
        {
            blockNumber = 0;
            print_str("\n\rblock number is too large.");
            print_str("\n\rEnter value < 4194304\n\r");
        }
        c = USART_Receive();
    }
    return blockNumber;
}
