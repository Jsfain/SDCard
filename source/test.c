/*************************************************************************
 * Author: Joshua Fain
 * Date:   6/27/2020
 * 
 * Contians main()
 * File used to test out the sd_test card functions.
 * **********************************************************************/

#include <stdint.h>
#include <avr/io.h>
#include "../includes/sd_base.h"
#include "../includes/usart.h"
#include "../includes/spi.h"
#include "../includes/prints.h"
#include "../includes/sd_misc.h"


//  *******   MAIN   ********  
int main(void)
{
    USART_Init();
    SPI_MasterInit();
    
    uint32_t initResponse;
    //attempt to initialize sd card.
    for(int i = 0; i<1; i++)
    {
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

    DataSector ds = sd_ReadSingleDataSector(0);
    print_sector(ds.byte);

    //just something to do after after running the sd card routines
    while(1)
        USART_Transmit(USART_Receive());
    
    return 0;
}