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
#include "../includes/sd_spi.h"
#include "../includes/usart.h"
#include "../includes/spi.h"
#include "../includes/prints.h"
#include "../includes/sd_misc.h"


//  *******   MAIN   ********  
int main(void)
{
    USART_Init();
    SPI_MasterInit();

    // Clear screen:
    for(int k=0;k<0xFF;k++) print_str("\n\r");

    print_str("\n\n\n\r ******  BEGIN TESTING *********\n\n\n\r");

    uint32_t initResponse;

    //attempt to initialize sd card.
    for(int i = 0; i<2; i++)
    {
        print_str("\n\n\rSD Card Initialization Attempt: ");
        print_dec(i);
        initResponse = sd_SPI_Mode_Init();
        if(initResponse != OUT_OF_IDLE) // If response is anything other than 0 (OUT_OF_IDLE) then initialization failed.
        {    
            print_str("\n\n\rSD INITIALIZATION FAILED with Response 0x");
            print_hex(initResponse);
            sd_printInitResponse(initResponse);
        }
        else
        {   print_str("\n\rSD Card Successfully Initialized\n\r");
            break;
        }
    }
    sd_printInitResponse(initResponse);
    

    if(initResponse==OUT_OF_IDLE) // initialization successful
    {      
        
        // ***** test sd_GetMemoryCapcity() function in sd_misc.c  *****
        uint32_t mc = sd_GetMemoryCapacity();
        print_str("\n\rmemory capacity = ");
        print_dec(mc);
        print_str(" Bytes\n\r");
        // ***** END test sd_GetMemoryCapcity()
        


        /*
        // ***** test sd_WriteSingleDataBlock() function in sd_misc.c *****
        
        DataBlock ds; //data block struct
        uint32_t block = 0;
        uint32_t address = block * DATA_BLOCK_LEN; // the address of first byte in block.
        uint16_t wr; // write response

        // data to write to block
        uint8_t db[DATA_BLOCK_LEN] = "HELLO WORLD......"; 
    
        //see what is currently written to block we will be writing to. 
        ds = sd_ReadSingleDataBlock(address);
        sd_PrintDataBlock(ds.data);

        print_str("\n\r *** WRITING SINGLE BLOCK *** \n\r");
        wr = sd_WriteSingleDataBlock(address,db);

        //Get R2 response (SEND_STATUS) if there is a write error.    
        if ((wr&0x0F00)==WRITE_ERROR)
        {
            print_str("\n\r Write error detected");
            uint16_t R2;
        
            CS_LOW;             
            sd_SendCommand(SEND_STATUS,0);
            R2 = sd_getR1(); // The first byte of the R2 response is same as the R1 response.
            R2 <<= 8;
            R2 |= sd_getR1(); // can use the sd_getR1 to get second byte of R2 response as well.
            print_dec(R2);
            CS_HIGH;
            print_str("\n\r R2 Response = ");
            print_dec(R2);
            print_str("\n\r Write Response = ");
            sd_PrintWriteError(wr);
        }
        
        else // No write error, then verify data has been written to block at address
        {
            ds = sd_ReadSingleDataBlock(address);
            sd_PrintDataBlock(ds.data);
        }
        */


        
        // ***** test sd_WriteSingleDataBlock() function in sd_misc.c *****

        //DataBlock ds; //data block struct
        uint32_t write_start_block = 0;
        uint32_t write_start_address = write_start_block * DATA_BLOCK_LEN; // the address of first byte in block.
        uint16_t mwr; // multiple write response

        int nowb = 3; // number of write blocks
        
        uint8_t mdb[DATA_BLOCK_LEN] = "Howdy Folks!";

        print_str("\n\r ***** Read Multiple Blocks *****");
        sd_PrintMultipleDataBlocks(write_start_address,nowb);
        
        print_str("\n\r **** Write Multiple Blocks *****");
        mwr = sd_WriteMultipleDataBlocks(write_start_address,nowb,mdb);

        //Get R2 response (SEND_STATUS) if there is a write error.    
        if ((mwr&0x0F00)==WRITE_ERROR)
        {
            print_str("\n\r Write error detected");
            uint16_t R2;
        
            CS_LOW;             
            sd_SendCommand(SEND_STATUS,0);
            R2 = sd_getR1(); // The first byte of the R2 response is same as the R1 response.
            R2 <<= 8;
            R2 |= sd_getR1(); // can use the sd_getR1 to get second byte of R2 response as well.
            print_dec(R2);
            CS_HIGH;
            print_str("\n\r R2 Response = ");
            print_dec(R2);
            print_str("\n\r Write Response = ");
            sd_PrintWriteError(mwr);
        }

        
        print_str("\n\rDone Writing Data Blocks");
        sd_PrintMultipleDataBlocks(write_start_address,nowb);
        

        // Gets the number of well written blocks
        uint32_t wwwb = 0;
        int count = 0;
        CS_LOW;
        sd_SendCommand(APP_CMD,0);
        sd_printR1(sd_getR1());
        sd_SendCommand(SEND_NUM_WR_BLOCKS,0);// number of well written write blocks
        print_str("\n\r");sd_printR1(sd_getR1());

        
        while(sd_ReturnByte() != 0xFE) // start block token
        {
            if(count++ > 0xFE) { print_str("\n\r timeout while waiting for start token"); break;}
        }
        if(count < 0xFE)
        {
            wwwb = sd_ReturnByte();
            wwwb <<= 8;
            wwwb |= sd_ReturnByte();
            wwwb <<= 8;
            wwwb |= sd_ReturnByte();
            wwwb <<= 8;
            wwwb |= sd_ReturnByte();
        }
        CS_HIGH;
        print_str("\n\r Number of well written write blocks = ");
        print_dec(wwwb);
        


        /*
        // ***** Test Erase Blocks *****
        int noeb = 3; // number of erase blocks
        uint32_t erase_start_block = 0;
        uint32_t erase_start_address = erase_start_block * DATA_BLOCK_LEN; // the address of first byte in block.
        uint32_t erase_end_address = noeb * DATA_BLOCK_LEN;

        uint16_t er; // erase response


        print_str("\n\r ***** Read Multiple Blocks *****");
        sd_PrintMultipleDataBlocks(erase_start_address,noeb);
        
        print_str("\n\r ***** Erase Multiple Blocks *****");
        er = sd_EraseBlocks(erase_start_address,erase_end_address);
        sd_PrintEraseBlockError(er);

        print_str("\n\r ***** Read Multiple Blocks *****");
        sd_PrintMultipleDataBlocks(erase_start_address,noeb);
        */        
    }

    // Something to do after SD card testing has completed.
    while(1)
        USART_Transmit(USART_Receive());
    
    return 0;
}