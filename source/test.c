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

    /*
    // test sd_GetMemoryCapcity() function in sd_misc.c
    uint32_t mc = sd_GetMemoryCapacity();
    print_str("\n\rmemory capacity = ");
    print_dec(mc);
    print_str("\n\r");
    */

    // Test sd_WriteSingleDataBlock, by writing data in db[] array.
    // This will first print the data block (sd_ReadSingleDataBlock and PrintSector)
    // Then write to the data block (sd_WriteSingelDataBlock) and then 
    // print the data block again to confirm a successful write operation has completed.
    
    
    //DataSector ds;

    uint32_t sector = 0;
    uint32_t address = 0;

    int nob = 3;
    
    uint8_t db[DATA_BLOCK_LEN] = "Helopoiu e";

    print_str("\n\r INITIAL BLOCK STATE\n\r");
    sd_PrintMultipleDataBlocks(address,nob);
    
    uint16_t wr;
    print_str("\n\r WRITING \n\r");
    for(int j = 0; j < nob; j++)
    {
        sector = 0 + j;
        address = sector * 512;
        wr = sd_WriteSingleDataBlock(address,db);

        //send status returns R2 response.  Should be called if there is a write error.
        
        if ((wr&0x0F00)==WRITE_ERROR)
        {
            uint16_t R2;
        
        
            CS_ASSERT;             
            sd_SendCommand(SEND_STATUS,0);
            R2 = sd_getR1(); // The first byte of the R2 response is the R1 response.
            R2 <<= 8;
            R2 |= sd_getR1(); // can use the sd_getR1 to get second byte of R2 response as well.
            print_dec(R2);
            CS_DEASSERT;
            print_str("\n\rR2 Response = ");
            print_dec(R2);
            print_str("\n\rWrite Response = ");
            sd_PrintWriteError(wr);
        }
    }
    
    address = 0;
    print_str("\n\r POST WRITE BLOCK STATE\n\r");
    sd_PrintMultipleDataBlocks(address,nob);
    

    address = (nob - 1) * 512;
    uint16_t err = sd_EraseSectors(0,address);
    print_str("\n\r POST ERASE BLOCK STATE\n\r");
    sd_PrintMultipleDataBlocks(address,nob);
    
    sd_PrintEraseSectorError(err);
    
    // Something to do after SD card testing has completed.
    while(1)
        USART_Transmit(USART_Receive());
    
    return 0;
}