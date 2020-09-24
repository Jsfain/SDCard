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
#include "../../../../general/includes/usart.h"
#include "../../../../general/includes/spi.h"
#include "../../../../general/includes/prints.h"
#include "../includes/sd_spi_base.h"
#include "../includes/sd_spi_data_access.h"
#include "../includes/sd_spi_misc.h"



uint32_t enterBlockNumber();


//  *******   MAIN   ********
int main(void)
{
    USART_Init();
    SPI_MasterInit();

    // Clear screen:
    for(int k=0;k<0xFF;k++) print_str("\n\r");

    print_str("\n\n\n\r ******  BEGIN TESTING *********\n\n\n\r");


    CardTypeVersion ctv = {.version = 0, .type = SDSC};
    print_str("\n\r ctv.version = "); print_dec(ctv.version);
    print_str("\n\r ctv.type    = "); 
    if(ctv.type == SDSC) print_str("SDSC");
    else if (ctv.type == SDHC) print_str("SDHC");

    uint32_t initResponse;

    //attempt to initialize sd card.
    for(int i = 0; i<1; i++)
    {
        print_str("\n\n\rSD Card Initialization Attempt: ");
        print_dec(i);
        initResponse = SD_InitializeSPImode(&ctv);
        if(initResponse != OUT_OF_IDLE) // If response is anything other than 0 (OUT_OF_IDLE) then initialization failed.
        {    
            print_str("\n\n\rSD INITIALIZATION FAILED with Response 0x");
            print_hex(initResponse);
            SD_PrintInitError(initResponse);
        }
        else
        {   print_str("\n\rSD Card Successfully Initialized\n\r");
            break;
        }
    }
    print_str("\n\rPrinting R1 Response for init = "); SD_PrintR1((uint8_t)(0x000FF&initResponse));
    print_str("\n\rinitialization response = "); SD_PrintInitError(initResponse);

    print_str("\n\r ctv.version = "); print_dec(ctv.version);
    print_str("\n\r ctv.type    = "); 
    if(ctv.type == SDSC) print_str("SDSC");
    else if (ctv.type == SDHC) print_str("SDHC");


    if(initResponse==OUT_OF_IDLE) // initialization successful
    {      
        // ***** test sd_GetMemoryCapcity() function in sd_misc.c  *****
        /*
        uint32_t mc = sd_GetMemoryCapacity();
        print_str("\n\rmemory capacity = ");
        print_dec(mc);
        print_str(" Bytes\n\r");
        */
        // ***** END test sd_GetMemoryCapcity()
        

        // **** test SD_SearchNonZeroBlocks()
        
        SD_SearchNonZeroBlocks(0,100);
        
        Block ds; //data block struct
        uint32_t block = 0;
        uint16_t err;
        uint32_t address = block * DATA_BLOCK_LEN; // the address of first byte in block.
        
        err = SD_ReadSingleDataBlock(0, &ds);
        print_str(" err = ");SD_PrintReadError(err);
        SD_PrintDataBlock(ds.byte);

        /*
        err = SD_ReadSingleDataBlock(1, &ds);
        print_str(" err = ");SD_PrintReadError(err);
        SD_PrintDataBlock(ds.byte);
        
        
        err = SD_ReadSingleDataBlock(16 * 512, &ds);
        print_str(" err = ");SD_PrintReadError(err);
        SD_PrintDataBlock(ds.byte);
        */
        // ***** test read/print multiple data block *******
        //USART_Receive();
        //int nob = 5;
        //uint32_t block = 0;
        //uint32_t address = block * DATA_BLOCK_LEN; // the address of first byte in block.   
        //SD_PrintMultipleDataBlocks(address,nob);
        


        /*
        // ***** test sd_WriteSingleDataBlock() function in sd_misc.c *****
        
        DataBlock ds; //data block struct
        uint32_t block = 0;
        uint32_t address = block * DATA_BLOCK_LEN; // the address of first byte in block.
        uint16_t wr; // write response

        // data to write to block
        uint8_t db[DATA_BLOCK_LEN] = "HELLO WORLD......"; 
    
        //see what is currently written to block we will be writing to. 
        ds = SD_ReadSingleDataBlock(address);
        SD_PrintDataBlock(ds.data);

        print_str("\n\r *** WRITING SINGLE BLOCK *** \n\r");
        wr = SD_WriteSingleDataBlock(address,db);

        //Get R2 response (SEND_STATUS) if there is a write error.    
        if ((wr&0x0F00)==WRITE_ERROR)
        {
            print_str("\n\r Write error detected");
            uint16_t r2;
        
            CS_LOW;             
            SD_SendCommand(SEND_STATUS,0);
            r2 = SD_getR1(); // The first byte of the R2 response is same as the R1 response.
            r2 <<= 8;
            r2 |= SD_getR1(); // can use the sd_getR1 to get second byte of R2 response as well.
            print_dec(r2);
            CS_HIGH;
            print_str("\n\r R2 Response = ");
            print_dec(r2);
            print_str("\n\r Write Response = ");
            SD_PrintWriteError(wr);
        }
        
        else // No write error, then verify data has been written to block at address
        {
            ds = SD_ReadSingleDataBlock(address);
            SD_PrintDataBlock(ds.data);
        }
        */


        
        /*
        // ***** test sd_WriteMultipleDataBlocks() function in sd_misc.c *****

        //DataBlock ds; //data block struct
        uint32_t write_start_block = 20;
        uint32_t write_start_address = write_start_block * DATA_BLOCK_LEN; // the address of first byte in block.
        uint16_t mwr; // multiple write response

        int nowb = 4; // number of write blocks
        
        uint8_t mdb[DATA_BLOCK_LEN] = "Well            Hello           There!          I               see             you             brought         a               PIE :) ";

        print_str("\n\r ***** Read Multiple Blocks *****");
        //SD_PrintMultipleDataBlocks(write_start_address,nowb);
        
        print_str("\n\r **** Write Multiple Blocks *****");
        mwr = SD_WriteMultipleDataBlocks(write_start_address,nowb,mdb);

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
        //SD_PrintMultipleDataBlocks(write_start_address,nowb);
        

        uint32_t nwwb = SD_NumberOfWellWrittenBlocks();
        print_str("\n\r Number of well written write blocks = ");
        print_dec(nwwb);
        // ***** END test SD_WriteMultipleDataBlocks() *****
        */


        /*
        // ***** Test Erase Blocks *****

        // calls SD_EraseBlocks(start_add, end_add) which will erase all blocks between the start and end address as
        // well as the entire block that contains the start and end address.  Start and end address do not need to be the 
        // first address of a block but must be contained within the first and last blocks to erase.

        uint32_t noeb = 400; // number of erase blocks
        uint32_t erase_start_block = 0;
        uint32_t erase_start_address = erase_start_block * DATA_BLOCK_LEN; // the address of first byte in block.
        uint32_t erase_end_address = erase_start_address + ((noeb - 1) * DATA_BLOCK_LEN); // "noeb-1 to ensure the noeb is the number of blocks erased"

        uint16_t er; // erase response

        print_dec(noeb*512);
        //print_str("\n\r ***** Read Multiple Blocks *****");
        //SD_PrintMultipleDataBlocks(erase_start_address,noeb);
        
        print_str("\n\r ***** Erase Multiple Blocks *****\n\r");
        er = sd_EraseBlocks(erase_start_address,erase_end_address);
        SD_PrintEraseBlockError(er);
        
        //print_str("\n\r ***** Read Multiple Blocks *****");
        //SD_PrintMultipleDataBlocks(erase_start_address,noeb+2);
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


            start_address = DATA_BLOCK_LEN * start_sector;
            err = SD_PrintMultipleDataBlocks(start_address,nos);
            //print_str("\n\rerr = 0x"); print_hex(err);
            SD_PrintReadError(err);

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