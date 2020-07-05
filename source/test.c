/*************************************************************************
 * Author: Joshua Fain
 * Date:   6/27/2020
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
            print_str("\n\n\rSD INITIALIZATION FAILED with Response, \n\r");
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
    DataSector ds;

    uint32_t sector = 0;
    uint32_t address = 0;

    sector = 0;
    address = sector * 512;

    ds = sd_ReadSingleDataBlock(address);
    sd_PrintSector(ds.data);

    uint8_t db[DATA_BLOCK_LEN] = "Hello World! What would you like to do today?";

    uint16_t wr = sd_WriteSingleDataBlock(0,db);
    print_hex(wr);
    sd_printWriteError(wr);
    

    ds = sd_ReadSingleDataBlock(address);
    sd_PrintSector(ds.data);



    // Something to do after SD card testing has completed.
    while(1)
        USART_Transmit(USART_Receive());
    
    return 0;
}