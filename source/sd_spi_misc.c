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
#include "..//includes/usart.h"
#include "../includes/spi.h"
#include "../includes/prints.h"



// Calculate and return the memory capacity 
// of a standard capacity SD Card in Bytes.
uint32_t SD_GetMemoryCapacitySC(void)
{
    uint8_t r1 = 0;

    // SEND_CSD (CMD9)
    CS_LOW;
    SD_SendCommand(SEND_CSD,0);
    r1 = SD_GetR1(); // Get R1 response
    if(r1 > 0) { CS_HIGH; return 1; }

    // ***** Get rest of the response bytes which are the CSD Register and CRC.
    
    // CSD fields
    uint8_t readBlockLength = 0;
    uint16_t cSize = 0;
    uint8_t cSizeMult = 0;

    // Other variables
    uint8_t resp;
    uint8_t timeout = 0;

    do{ // CSD_STRUCTURE - Must be 0 for SDSC types.
        if( (SD_ReceiveByteSPI() >> 6) == 0 ) break;
        if(timeout++ >= 0xFF){ CS_HIGH; return 1; }
    }while(1);

    timeout = 0;
    do{ // TAAC - Bit 7 is reserved so must be 0
        if(!(SD_ReceiveByteSPI()>>7)) break;
        if(timeout++ >= 0xFF) { CS_HIGH; return 1;}
    }while(1);

    // NSAC - Any value could be here
    SD_ReceiveByteSPI();

    timeout = 0;
    do{ // TRAN_SPEED
        resp = SD_ReceiveByteSPI();  
        if((resp == 0x32) || (resp == 0x5A)) break;
        if(timeout++ >= 0xFF) { CS_HIGH; return 1;}
    }while(1);


    // Next 2 bytes are the 12-bit CCC and 4-bit READ_BL_LEN. CCC is of the
    // form 01_110110101. READ_BL_LEN is needed to calculate memory capacity.
    // It's value must be 9, 10, or 11.
    uint8_t flag = 1;
    while(flag == 1)
    {
        if((resp = SD_ReceiveByteSPI())&0x7B) //CCC[11:4] 
        {
            if(((resp = SD_ReceiveByteSPI())>>4) == 0x05) //CCC[3:0]
            {
                readBlockLength = resp & 0b00001111;
                if ((readBlockLength < 9) || (readBlockLength > 11)) 
                    { CS_HIGH; return 1; }
                flag = 0;
            }
        }
        if(timeout++ >= 0xFF) { CS_HIGH; return 1;}
    }


    // Gets the remaining fields needed: C_SIZE and C_SIZE_MULT
    flag = 1;
    timeout = 0;
    while(flag == 1)
    {
        if((resp = SD_ReceiveByteSPI()) & 0xF3) //READ_BLK_PARTIAL[7]   = 1;
                                                //WRITE_BLK_MISALIGN[6] = X;
                                                //READ_BLK_MISALIGN[5]  = X;
                                                //DSR_IMP[4] = X;
                                                //RESERVERED[3:2] = 0;
        {           
            cSize = (resp & 0x03);         
            cSize <<= 8;           
            cSize |= SD_ReceiveByteSPI();
            cSize <<= 2;        
            cSize |= (SD_ReceiveByteSPI() >> 6);
            
            cSizeMult = ( (SD_ReceiveByteSPI() & 0x03) << 1);
            cSizeMult |= (SD_ReceiveByteSPI() >> 7);
            
            flag = 0;
        }
        if(timeout++ >= 0xFF) { CS_HIGH; return 1; }
    }

    CS_HIGH;

    // ******* Calculate memory capacity ******
    
    // BLOCK_LEN = 2^READ_BL_LEN
    uint32_t blockLen = 1;
    for (uint8_t i = 0; i < readBlockLength; i++) blockLen = blockLen * 2;

    // MULT = 2^(C_SIZE_MULT + 2)
    uint32_t mult = 1;
    for (uint8_t i = 0; i < cSizeMult+2; i++) mult = mult * 2;

    uint32_t blockNr = (cSize + 1) * mult;

    uint32_t memoryCapacity = blockNr * blockLen;
    
    return memoryCapacity; //bytes
}
// END sd_MemoryCapacity()



// Calculate memory capacity of a high capacity SD Card
// from CSD and and return the memory capacity in Bytes.
uint32_t SD_GetMemoryCapacityHC(void)
{
    uint8_t r1 = 0;

    // SEND_CSD (CMD9)
    CS_LOW;
    SD_SendCommand(SEND_CSD,0);
    r1 = SD_GetR1(); // Get R1 response
    if(r1 > 0) { CS_HIGH; return 1; }
    

    // Only C_SIZE value is needed to calculate the memory capacity of a SDHC/
    // SDXC type card. The CSD fields prior to the C_SIZE are used here to 
    // verfiy CSD is accurately being read-in, and the correct memory capacity
    // function has been called.

    uint8_t resp;
    uint8_t timeout = 0;
    uint64_t cSize = 0;

    do{ // CSD_STRUCTURE - Must be 1 for SDHC types.
        if( (resp = (SD_ReceiveByteSPI() >> 6)) == 1 ) break;
        if(timeout++ >= 0xFF){ CS_HIGH; return 1; }
    }while(1);
    
    timeout = 0;
    do{ // TAAC - Must be 0X0E (1ms)
        if( (resp = SD_ReceiveByteSPI()) == 0x0E) break;
        if(timeout++ >= 0xFF) { CS_HIGH; return 1; }
    }while(1);

    timeout = 0;
    do{ // NSAC - Must be 0X00 (1ms)
        if( (resp = SD_ReceiveByteSPI()) == 0x00) break;
        if(timeout++ >= 0xFF) { CS_HIGH; return 1; }
    }while(1);

    timeout = 0;
    do{ // TRAN_SPEED - Must be 0x32 for current implementation
        if( (resp = SD_ReceiveByteSPI()) == 0x32)  break;
        if(timeout++ >= 0xFF) { CS_HIGH; return 1; }
    }while(1);

    // Next 2 bytes are the 12-bit CCC and 4-bit READ_BL_LEN. CCC is of the
    // form _1_1101101_1 for SDHC/SDXC. READ_BL_LEN must be 9 for SDHC/SDXC.
    uint8_t flag = 1;
    while(flag == 1)
    {
        if( ( (resp = SD_ReceiveByteSPI()) & 0x5B) == 0x5B) //CCC[11:4]
        {
            if(((resp = SD_ReceiveByteSPI()) >> 4) == 0x05) //CCC[3:0] 
            {
                if ( (resp & 0x0F) != 9) { CS_HIGH; return 1; }
                flag = 0;
            }
        }
        if(timeout++ >= 0xFF) { CS_HIGH; return 1;}
    }

    //This section gets the remaining bits leading up to C_SIZE.
    flag = 1;
    timeout = 0;

    while(flag == 1)
    {
        resp = SD_ReceiveByteSPI();
        if( (resp == 0) || (resp == 16) ) //READ_BLK_PARTIAL[7] = 0;
                                          //WRITE_BLK_MISALIGN[6] = 0;
                                          //READ_BLK_MISALIGN[5] = 0;
                                          //DSR_IMP[4] = X;
                                          //RESERVERED[3:0] = 0;
        {
            cSize = (SD_ReceiveByteSPI() & 0x3F); // Only [5:0] is C_SIZE           
            cSize <<= 8;           
            cSize |= SD_ReceiveByteSPI();
            cSize <<= 8;        
            cSize |= SD_ReceiveByteSPI();
            flag = 0;
        }
        if(timeout++ >= 0xFF) { CS_HIGH; return 1; }
    }
    CS_HIGH;

    uint32_t memoryCapacity = (cSize + 1) * 512000;
    
    return memoryCapacity; //bytes
}
// END sd_MemoryCapacity()



// Prints the block number of all blocks that have any non-zero 
// bytes between startBlockAddress and endBlockAddress.
void SD_FindNonZeroBlockNumbers(
                uint32_t startBlockAddress, 
                uint32_t endBlockAddress)
{
    //Block ds;
    uint8_t ds[512];
    uint16_t tab = 0; //used for printing format
    uint32_t address = 0;

    for( uint32_t blockNumber = startBlockAddress;
                  blockNumber <= endBlockAddress;
                  blockNumber++ )
    {
        address = blockNumber;
        SD_ReadSingleBlock(address, ds);       
        
        for(int i = 0; i < BLOCK_LEN; i++)
        {
            if(ds[i]!=0)
            {
                if(tab%5==0) print_str("\n\r");
                print_str("\t\t");print_dec(blockNumber);
                tab++;
                break;
            }
        }
    }
}
// END sd_SearchNonZeroDataBlocks()
