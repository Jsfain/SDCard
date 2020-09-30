/*************************************************************************
 * Author: Joshua Fain
 * Date:   7/5/2020
 * 
 * Contians main()
 * File used to test out the sd card functions. Changes regularly
 * depending on what is being tested.
 * **********************************************************************/

#include <stdint.h>
#include <avr/io.h>
#include "../includes/usart.h"
#include "../includes/spi.h"
#include "../includes/prints.h"
#include "../includes/sd_spi_base.h"
#include "../includes/sd_spi_data_access.h"
#include "../includes/sd_spi_misc.h"


uint32_t enterBlockNumber();



int main(void)
{
    USART_Init();
    SPI_MasterInit();


    // ******************* SD CARD INITILIAIZATION ***************** 
    // Initializing ctv. These members will be set by the SD card's
    // initialization routine. They should only be set there.
    CardTypeVersion ctv = {.version = 1, .type = SDSC};

    uint32_t initResponse;

    // Attempt, up to 10 times, to initialize the SD card.
    for(int i = 0; i < 10; i++)
    {
        print_str("\n\n\rSD Card Initialization Attempt # "); print_dec(i);
        initResponse = SD_InitializeSPImode(&ctv);
        if( ( (initResponse & 0xFF) != OUT_OF_IDLE) && 
            ( (initResponse & 0xFFF00) != 0 ) )
        {    
            print_str("\n\n\rFAILED INITIALIZING SD CARD");
            print_str("\n\rInitialization Error Response: "); 
            SD_PrintInitError(initResponse);
            print_str("\n\rR1 Response: "); SD_PrintR1(initResponse);
        }
        else
        {   print_str("\n\rSUCCESSFULLY INITIALIZED SD CARD");
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


        Block ds; // block struct
        uint16_t err16; // returned error values
        
        
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

        startBlockNumber = 3001;
        endBlockNumber = 4000;

        if (ctv.type == SDHC) // SDHC is block addressable
            SD_FindNonZeroBlockNumbers(startBlockNumber, endBlockNumber);
        else // SDSC byte addressable
            SD_FindNonZeroBlockNumbers( 
                        startBlockNumber * BLOCK_LEN, 
                        endBlockNumber * BLOCK_LEN);

        // *****************************************************************


/*
        // ******************* SD_ReadSingleBlock() ************************
        // 
        // Read in a single block from the SD card specified by blockNumber 
        // into the array member of ds. This also demonstrates how to use 
        // SD_PrintBlock() to print the block just read into the array.

        blockNumber = 8192;
        if (ctv.type == SDHC) // SDHC is block addressable
            err16 = SD_ReadSingleBlock(blockNumber, &ds);
        else // SDSC byte addressable
            err16 = SD_ReadSingleBlock(blockNumber * BLOCK_LEN, &ds);
        
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
            SD_PrintBlock(ds.byte);

        // **********************************************************
        
*/
/*

        // ******************* SD_PrintMultipleBlocks() **********************
        //
        // Use this function to print multiple SD card blocks back-to-back
        // beginning at blockNumber. The number of blocks printed is specified
        // numberOfBlocks. 
        // This function calls the READ_MULTIPLE_BLOCKS SD card command.
        
        uint32_t numberOfBlocks = 3;
        
        blockNumber = 8192;
          
        //SD_PrintMultipleBlocks(address,nob);
        

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
        // ***************************************************************
*/        



        // ******************* SD_WriteSingleBlock() **************************
        //
        // Use this function to write to a single block on the SD card 
        // specified by blockNumber. This demo will first erase a single block
        // using SD_EraseBlocks(), read and print the same block using
        // SD_ReadSingleBlock and SD_PrintBlock() to show successful erase. It
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

        // READ and PRINT
        if (ctv.type == SDHC) // SDHC is block addressable
            err16 = SD_ReadSingleBlock(blockNumber, &ds);
        else // SDSC byte addressable
            err16 = SD_ReadSingleBlock(blockNumber * BLOCK_LEN, &ds);

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
            SD_PrintBlock(ds.byte);

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
                err16 = SD_ReadSingleBlock(blockNumber, &ds);
            else // SDSC byte addressable
                err16 = SD_ReadSingleBlock(blockNumber * BLOCK_LEN, &ds);

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
                SD_PrintBlock(ds.byte);
        }
        


        
        /*
        // ***** test SD_WriteMultipleBlocks() function in sd_misc.c *****

        //DataBlock ds; //data block struct
        uint32_t write_start_block = 20;
        uint32_t write_start_address = write_start_block * BLOCK_LEN; // the address of first byte in block.
        uint16_t mwr; // multiple write response

        int nowb = 4; // number of write blocks
        
        uint8_t mdb[BLOCK_LEN] = "Well            Hello           There!          I               see             you             brought         a               PIE :) ";

        print_str("\n\r ***** Read Multiple Blocks *****");
        //SD_PrintMultipleBlocks(write_start_address,nowb);
        
        print_str("\n\r **** Write Multiple Blocks *****");
        mwr = SD_WriteMultipleBlocks(write_start_address,nowb,mdb);

        //Get R2 response (SEND_STATUS) if there is a write error.    
        if ((mwr&0x0F00)==WRITE_ERROR)
        {
            print_str("\n\r Write error detected");
            uint16_t R2;
        
            CS_LOW;             
            SD_SendCommand(SEND_STATUS,0);
            R2 = SD_GetR1(); // The first byte of the R2 response is same as the R1 response.
            R2 <<= 8;
            R2 |= SD_GetR1(); // can use the sd_getR1 to get second byte of R2 response as well.
            print_dec(R2);
            CS_HIGH;
            print_str("\n\r R2 Response = ");
            print_dec(R2);
            print_str("\n\r Write Response = ");
            SD_PrintWriteError(mwr);
        }
        
        
        print_str("\n\rDone Writing Data Blocks");
        //SD_PrintMultipleBlocks(write_start_address,nowb);
        

        uint32_t nwwb = SD_NumberOfWellWrittenBlocks();
        print_str("\n\r Number of well written write blocks = ");
        print_dec(nwwb);
        // ***** END test SD_WriteMultipleBlocks() *****
        */


        /*
        // ***** Test Erase Blocks *****

        // calls SD_EraseBlocks(start_add, end_add) which will erase all blocks between the start and end address as
        // well as the entire block that contains the start and end address.  Start and end address do not need to be the 
        // first address of a block but must be contained within the first and last blocks to erase.

        uint32_t noeb = 400; // number of erase blocks
        uint32_t erase_start_block = 0;
        uint32_t erase_start_address = erase_start_block * BLOCK_LEN; // the address of first byte in block.
        uint32_t erase_end_address = erase_start_address + ((noeb - 1) * BLOCK_LEN); // "noeb-1 to ensure the noeb is the number of blocks erased"

        uint16_t er; // erase response

        print_dec(noeb*512);
        //print_str("\n\r ***** Read Multiple Blocks *****");
        //SD_PrintMultipleBlocks(erase_start_address,noeb);
        
        print_str("\n\r ***** Erase Multiple Blocks *****\n\r");
        er = sd_EraseBlocks(erase_start_address,erase_end_address);
        SD_PrintEraseBlockError(er);
        
        //print_str("\n\r ***** Read Multiple Blocks *****");
        //SD_PrintMultipleBlocks(erase_start_address,noeb+2);
        // ***** END Test Erase Blocks *****
        */
       
        uint32_t start_sector;
        uint32_t start_address;
        uint32_t nos;
        uint8_t answer;
        //uint16_t err;
        do{
            do{
                print_str("\n\rEnter Start Sector\n\r");
                start_sector = enterBlockNumber();
                print_str("\n\rhow many sectors do you want to print?\n\r");
                nos = enterBlockNumber();
                print_str("\n\rYou want to print "); print_dec(nos);
                print_str(" sectors beginning at sector "); print_dec(start_sector);
                print_str("\n\ris this correct? (y/n)");
                answer = USART_Receive();
                USART_Transmit(answer);
                print_str("\n\r");
            }while(answer != 'y');


            start_address = BLOCK_LEN * start_sector;
            err16 = SD_PrintMultipleBlocks(start_address,nos);
            //print_str("\n\rerr = 0x"); print_hex(err);
            SD_PrintReadError(err16);

            print_str("\n\r press 'q' to quit and any other key to go again: ");
            answer = USART_Receive();
            USART_Transmit(answer);
        }while(answer != 'q');
    }

    // Something to do after SD card testing has completed.
    while(1)
        USART_Transmit(USART_Receive());
    
    return 0;
}



uint32_t enterBlockNumber()
{
    uint8_t x;
    uint8_t c;
    uint32_t sector = 0;

    //print_str("Specify data block (data block = 512 bytes) and press enter\n\r");
    c = USART_Receive();
    
    while(c!='\r')
    {
        x = c - '0';
        sector = sector*10;
        sector += x;
        print_str("\r");
        print_dec(sector);
        c = USART_Receive();
        if(sector >= 4194304)
        {
            sector = 0;
            print_str("sector value is too large.\n\rEnter value < 4194304\n\r");
        }
    }
    return sector;
}
