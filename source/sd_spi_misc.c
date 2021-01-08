/*
*******************************************************************************
*                                  AVR-SD CARD MODULE
*
* File    : SD_SPI_MISC.C
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
* License : MIT
* Copyright (c) 2020
* 
*
* DESCRIPTION:
* This is meant to be a catch-all for some SD card functions that do not really
* fit into any of the other AVR-SD Card module files. These functions require 
* SD_SPI_BASE.C/H and SD_SPI_DATA_ACCESS.C/H
*******************************************************************************
*/


#include <stdint.h>
#include <avr/io.h>
#include "../includes/sd_spi_base.h"
#include "../includes/sd_spi_data_access.h"
#include "../includes/sd_spi_misc.h"
#include "..//includes/usart.h"
#include "../includes/spi.h"
#include "../includes/prints.h"





/*
*******************************************************************************
*******************************************************************************
 *                     
 *                               FUNCTIONS   
 *  
*******************************************************************************
*******************************************************************************
*/

/*
-------------------------------------------------------------------------------
|                          GET SDSC MEMORY CAPACITY
|                                        
| Description : Calculates and returns the memory capacity of a standard 
|               capacity SD Card (SDSC)
|
| Return      : The memory capacity of the SD card in bytes.
-------------------------------------------------------------------------------
*/

uint32_t 
sd_getMemoryCapacitySDSC (void)
{
    uint8_t r1 = 0;

    // SEND_CSD (CMD9)
    CS_SD_LOW;
    sd_sendCommand (SEND_CSD,0);
    r1 = sd_getR1(); // Get R1 response
    if(r1 > 0) { CS_SD_HIGH; return 1; }

    // ***** Get rest of the response bytes which are the CSD Register and CRC.
    
    // CSD fields
    uint8_t readBlockLength = 0;
    uint16_t cSize = 0;
    uint8_t cSizeMult = 0;

    // Other variables
    uint8_t resp;
    uint8_t timeout = 0;

    do{ // CSD_STRUCTURE - Must be 0 for SDSC types.
        if( (sd_receiveByteSPI() >> 6) == 0 ) break;
        if(timeout++ >= 0xFF){ CS_SD_HIGH; return 1; }
    }while(1);

    timeout = 0;
    do{ // TAAC - Bit 7 is reserved so must be 0
        if(!(sd_receiveByteSPI()>>7)) break;
        if(timeout++ >= 0xFF) { CS_SD_HIGH; return 1;}
    }while(1);

    // NSAC - Any value could be here
    sd_receiveByteSPI();

    timeout = 0;
    do{ // TRAN_SPEED
        resp = sd_receiveByteSPI();  
        if((resp == 0x32) || (resp == 0x5A)) break;
        if(timeout++ >= 0xFF) { CS_SD_HIGH; return 1;}
    }while(1);


    // Next 2 bytes are the 12-bit CCC and 4-bit READ_BL_LEN. CCC is of the
    // form 01_110110101. READ_BL_LEN is needed to calculate memory capacity.
    // It's value must be 9, 10, or 11.
    uint8_t flag = 1;
    while(flag == 1)
    {
        if((resp = sd_receiveByteSPI())&0x7B) //CCC[11:4] 
        {
            if(((resp = sd_receiveByteSPI())>>4) == 0x05) //CCC[3:0]
            {
                readBlockLength = resp & 0b00001111;
                if ((readBlockLength < 9) || (readBlockLength > 11)) 
                    { CS_SD_HIGH; return 1; }
                flag = 0;
            }
        }
        if(timeout++ >= 0xFF) { CS_SD_HIGH; return 1;}
    }


    // Gets the remaining fields needed: C_SIZE and C_SIZE_MULT
    flag = 1;
    timeout = 0;
    while(flag == 1)
    {
        if((resp = sd_receiveByteSPI()) & 0xF3) //READ_BLK_PARTIAL[7]   = 1;
                                                //WRITE_BLK_MISALIGN[6] = X;
                                                //READ_BLK_MISALIGN[5]  = X;
                                                //DSR_IMP[4] = X;
                                                //RESERVERED[3:2] = 0;
        {           
            cSize = (resp & 0x03);         
            cSize <<= 8;           
            cSize |= sd_receiveByteSPI();
            cSize <<= 2;        
            cSize |= (sd_receiveByteSPI() >> 6);
            
            cSizeMult = ( (sd_receiveByteSPI() & 0x03) << 1);
            cSizeMult |= (sd_receiveByteSPI() >> 7);
            
            flag = 0;
        }
        if(timeout++ >= 0xFF) { CS_SD_HIGH; return 1; }
    }

    CS_SD_HIGH;

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



/*
-------------------------------------------------------------------------------
|                         GET SDHC / SDXC MEMORY CAPACITY
|                                        
| Description : Calculates and returns the memory capacity of a high/extended
|               capacity SD Card (SDHC/SDXC)
|
| Return      : The memory capacity of the SD card in bytes.
-------------------------------------------------------------------------------
*/

uint32_t 
sd_getMemoryCapacitySDHC (void)
{
    uint8_t r1 = 0;

    // SEND_CSD (CMD9)
    CS_SD_LOW;
    sd_sendCommand (SEND_CSD,0);
    r1 = sd_getR1(); // Get R1 response
    if (r1 > 0) 
      CS_SD_HIGH; return 1;
    

    // Only C_SIZE value is needed to calculate the memory capacity of a SDHC/
    // SDXC type card. The CSD fields prior to the C_SIZE are used here to 
    // verfiy CSD is accurately being read-in, and the correct memory capacity
    // function has been called.

    uint8_t resp;
    uint8_t timeout = 0;
    uint64_t cSize = 0;

    do
      { // CSD_STRUCTURE - Must be 1 for SDHC types.
        if ((resp = (sd_receiveByteSPI() >> 6)) == 1) 
          break;
        if (timeout++ >= 0xFF)
          { 
            CS_SD_HIGH; 
            return 1; 
          }
      }
    while(1);
    
    timeout = 0;
    do{ // TAAC - Must be 0X0E (1ms)
        if( (resp = sd_receiveByteSPI()) == 0x0E) break;
        if(timeout++ >= 0xFF) { CS_SD_HIGH; return 1; }
    }while(1);

    timeout = 0;
    do{ // NSAC - Must be 0X00 (1ms)
        if( (resp = sd_receiveByteSPI()) == 0x00) break;
        if(timeout++ >= 0xFF) { CS_SD_HIGH; return 1; }
    }while(1);

    timeout = 0;
    do{ // TRAN_SPEED - Must be 0x32 for current implementation
        if( (resp = sd_receiveByteSPI()) == 0x32)  break;
        if(timeout++ >= 0xFF) { CS_SD_HIGH; return 1; }
    }while(1);

    // Next 2 bytes are the 12-bit CCC and 4-bit READ_BL_LEN. CCC is of the
    // form _1_1101101_1 for SDHC/SDXC. READ_BL_LEN must be 9 for SDHC/SDXC.
    uint8_t flag = 1;
    while(flag == 1)
    {
        if( ( (resp = sd_receiveByteSPI()) & 0x5B) == 0x5B) //CCC[11:4]
        {
            if(((resp = sd_receiveByteSPI()) >> 4) == 0x05) //CCC[3:0] 
            {
                if ( (resp & 0x0F) != 9) { CS_SD_HIGH; return 1; }
                flag = 0;
            }
        }
        if(timeout++ >= 0xFF) { CS_SD_HIGH; return 1;}
    }

    //This section gets the remaining bits leading up to C_SIZE.
    flag = 1;
    timeout = 0;

    while(flag == 1)
    {
        resp = sd_receiveByteSPI();
        if( (resp == 0) || (resp == 16) ) //READ_BLK_PARTIAL[7] = 0;
                                          //WRITE_BLK_MISALIGN[6] = 0;
                                          //READ_BLK_MISALIGN[5] = 0;
                                          //DSR_IMP[4] = X;
                                          //RESERVERED[3:0] = 0;
        {
            cSize = (sd_receiveByteSPI() & 0x3F); // Only [5:0] is C_SIZE           
            cSize <<= 8;           
            cSize |= sd_receiveByteSPI();
            cSize <<= 8;        
            cSize |= sd_receiveByteSPI();
            flag = 0;
        }
        if(timeout++ >= 0xFF) { CS_SD_HIGH; return 1; }
    }
    CS_SD_HIGH;

    uint32_t memoryCapacity = (cSize + 1) * 512000;
    
    return memoryCapacity; //bytes
}



/*
-------------------------------------------------------------------------------
|                         GET SDHC / SDXC MEMORY CAPACITY
|                                        
| Description : Search consecutive blocks over a specified range for those that
|               contain any non-zero values. The number/address of any blocks
|               found, with data will be printed to a screen. This function is
|               not fast, so searching over a large range of blocks can take a
|               while.
|
| Arguments   : startBlckAddr  - address of the first block to search.
|             : endBlckAddr    - address of the last block to search.
-------------------------------------------------------------------------------
*/

void 
sd_findNonZeroDataBlockNums (uint32_t startBlckAddr, uint32_t endBlckAddr)
{
    uint8_t ds[512];
    uint16_t tab = 0; //used for printing format
    uint32_t address = 0;

    for( uint32_t blockNumber = startBlckAddr;
                  blockNumber <= endBlckAddr;
                  blockNumber++ )
    {
        address = blockNumber;
        sd_readSingleBlock (address, ds);       
        
        for(int i = 0; i < BLOCK_LEN; i++)
        {
            if(ds[i]!=0)
            {
                if(tab%5==0) print_str("\n\r");
                print_str ("\t\t"); print_dec (blockNumber);
                tab++;
                break;
            }
        }
    }
}