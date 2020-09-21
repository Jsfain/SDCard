 /*****************************************************************************
 * SD_MISC.C 
 * 
 * TARGET
 * ATmega 1280
 *
 * DESCRIPTION
 * Specialized functions for interacting with an SD card hosted on an AVR 
 * microcontroller operating in SPI mode. Uses SD_SPI_BASE.C(H) for physical
 * interface to the SD card.
 * 
 *
 * Author: Joshua Fain
 * Date:   9/17/2020
 * ***************************************************************************/



#include <stdint.h>
#include <avr/io.h>
#include "../includes/sd_spi_base.h"
#include "../includes/sd_spi_data_access.h"
#include "../includes/sd_spi_misc.h"
#include "../../../../general/includes/usart.h"
#include "../../../../general/includes/spi.h"
#include "../../../../general/includes/prints.h"



// Calculate and return the memory capacity of the SD Card in Bytes.
uint32_t sd_GetMemoryCapacity(void)
{
    //Initialize parameter values needed for memory capacity calculation.
    uint8_t READ_BL_LEN = 0;
    uint16_t C_SIZE = 0;
    uint8_t C_SIZE_MULT = 0;

    //Initialize other variables
    uint8_t resp;
    uint8_t R1 = 0;
    uint8_t attempt = 0;

    // ***** Send command SEND_CSD (CMD9) and get R1 response.
    CS_LOW;
    SD_SendCommand(SEND_CSD,0); //CMD9 - Request CSD Register from SD card.
    R1 = SD_GetR1(); // Get R1 response

    //If R1 is non-zero or times out, then return without getting rest of CSD.
    if(R1 > 0)
    {
        CS_HIGH
        print_str("\n\r>> ERROR:   SEND_CSD (CMD9) in sd_getMemoryCapacity() returned error in R1 response.");
        return 1;
    }

    
    // ***** Get rest of the response bytes which are the CSD Register and CRC.
    
    
    attempt = 0;
    do{ // get CSD_STRUCTURE (Not used for memory calculation)
        if((SD_ReceiveByteSPI()>>6) <= 2) break; //check for valid CSD_STRUCTURE returned
        if(attempt++ >= 0xFF){ CS_HIGH; return 1;} // Timeout returning valid CSD_STRUCTURE 
    }while(1);
    
    attempt = 0;
    do{ // get TAAC (Not used for memory calculation)
        if(!(SD_ReceiveByteSPI()>>7)) break; // check for valid TAAC (bit 7 is rsvd should = 0)
        if(attempt++ >= 0xFF) { CS_HIGH; return 1;} //Timeout returning valid TAAC
    }while(1);

    SD_ReceiveByteSPI();  // get NSAC of CSD. Any value could be valid. (Not used for memory calculation)

    attempt = 0;
    do{ // get TRAN_SPEED. (Not used for memory calculation)
        resp = SD_ReceiveByteSPI();  
        if((resp == 0x32) || (resp == 0x5A)) break; //TRAN_SPEED must be 0x32 or 0x5A.
        if(attempt++ >= 0xFF) { CS_HIGH; return 1;} //Timeout returning valid TRAN_SPEED
    }while(1);


    //Next 2 bytes include 12 bit CCC and 4 bit READ_BL_LENGTH.  
    //CCC is of the form 01_110110101 and is NOT used for memory calculation
    //READ_BL_LEN is needed to calculate memory capacity and must by 9, 10, or 11.
    uint8_t flag = 1;
    while(flag == 1)
    {
        if((resp = SD_ReceiveByteSPI())&0x7B) //CCC 8 most significant bits must be of the form 01_11011;
        {
            if(((resp = SD_ReceiveByteSPI())>>4) == 0x05) //CCC least sig. 4 bits and 4 bit READ_BL_LEN.
            {
                READ_BL_LEN = resp&0b00001111;
                if ((READ_BL_LEN < 9) || (READ_BL_LEN > 11)) { CS_HIGH; return 1; }
                flag = 0;
            }
        }
        if(attempt++ >= 0xFF) { CS_HIGH; return 1;}
    }


    //This section gets the remaining bits of the CSD.
    //C_SIZE and C_SIZE_MULT are needed for memory capacity calculation.
    flag = 1;
    attempt = 0;
    while(flag == 1)
    {
        if((resp = SD_ReceiveByteSPI())&0xF3) //MASK of 0xF3 is based on the following. X is 1 or 0.
                                        //READ_BLK_PARTIAL[7] = 1;
                                        //WRITE_BLK_MISALIGN[6] = X;
                                        //READ_BLK_MISALIGN[5] = X;
                                        //DSR_IMP[4] = X;
                                        //RESERVERED[3:2] = 0;
                                        //C_SIZE - 2 Most Significant Bits[1:0] = X.  (Used for memory capacity calculation.)
        {
            //get and parse C_SIZE            
            C_SIZE = (resp&0x03);           
            C_SIZE <<= 8;           
            C_SIZE |= SD_ReceiveByteSPI(); //get next 8 bits of C_SIZE
            C_SIZE <<= 2;        
            C_SIZE |= ((SD_ReceiveByteSPI())>>6);
            
            //get C_SIZE_MULT
            C_SIZE_MULT = ((SD_ReceiveByteSPI()&0x03)<<1);
            C_SIZE_MULT |= (SD_ReceiveByteSPI()>>7);
            
            flag = 0;
        }
        if(attempt++ >= 0xFF) { CS_HIGH; return 1; }
    }

    CS_HIGH;

    // ***** Calculate and return memory capacity of SD Card.
    uint32_t BLOCK_LEN = 1;
    for (uint8_t i = 0; i< READ_BL_LEN; i++) BLOCK_LEN = BLOCK_LEN*2;
    uint32_t MULT = 1;
    for (uint8_t i = 0; i< C_SIZE_MULT+2; i++) MULT = MULT*2;
    uint32_t BLOCKNR = (C_SIZE+1)*MULT;
    uint32_t memoryCapacity = BLOCKNR*BLOCK_LEN;
    return memoryCapacity; //bytes
}
//END sd_MemoryCapacity()

