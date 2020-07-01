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

    print_str("\n\n\n\r ******  BEGIN TESTING *********\n\n\n\r");
    for(int i=0;i<0xFF;i++) SPI_MasterTransmit(0xFF);

    uint32_t initResponse;
    //attempt to initialize sd card.
    for(int i = 0; i<2; i++)
    {
        print_str("\n\n\rSD Card Initialization Attempt: ");
        print_dec(i);
        initResponse = sd_SPI_Mode_Init();
        if(initResponse != OUT_OF_IDLE) // Response is anything other than anything other than 0
                                        // then initialization failed.
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


    print_dec(sd_getMemoryCapacity());

    DataSector ds;

    uint32_t sector = 0;
    uint32_t address = 0;
    /*
    for(int i = 0; i < 1; i++)
    {
        sector = 2048+i;
        address = sector * 512;
        ds = sd_ReadSingleDataSector(address);
        print_sector(ds.data);
    }
    */
    sector = 2048;
    address = sector * 512;
    ds = sd_ReadSingleDataBlock(address);
    print_sector(ds.data);


    //just something to do after after running the sd card routines
    while(1)
        USART_Transmit(USART_Receive());
    
    return 0;
}