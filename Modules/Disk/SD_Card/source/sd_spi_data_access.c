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
 * FUNCTION LIST
 * 1) DataBlock SD_ReadSingleDataBlock(uint32_t address)
 * 2) void      SD_PrintDataBlock(uint8_t *block)
 * 3) void      SD_PrintMultipleDataBlocks(
 *                      uint32_t start_address,
 *                      uint32_t numOfBlocks)
 * 4) void      SD_SearchNonZeroBlocks(
 *                      uint32_t begin_block,
 *                      uint32_t end_block)
 * 5) uint16_t  SD_WriteSingleDataBlock(
 *                      uint32_t address, 
 *                      uint8_t *dataBuffer)
 * 6) uint16_t  SD_WriteMultipleDataBlocks(
 *                      uint32_t address, 
 *                      uint8_t nob, 
 *                      uint8_t *dataBuffer)
 * 7) void      SD_PrintWriteError(uint16_t err)
 * 8) uint32_t  SD_NumberOfWellWrittenBlocks(void)
 * 9) uint16_t  SD_EraseBlocks(
 *                      uint32_t start_address,
 *                      uint16_t end_address)           
 * 10)void      SD_PrintEraseBlockError(uint16_t err)
 * 
 * 
 * Author: Joshua Fain
 * Date:   9/17/2020
 * ***************************************************************************/



#include <stdint.h>
#include <avr/io.h>
#include "../../../../general/includes/usart.h"
#include "../../../../general/includes/spi.h"
#include "../../../../general/includes/prints.h"
#include "../includes/sd_spi_base.h"
#include "../includes/sd_spi_data_access.h"



// Read in a single block (512 bytes) to a  from an SD card
uint16_t SD_ReadSingleDataBlock(uint32_t address, Block *ds)
{
    uint8_t timeout = 0;
    uint8_t r1;

    CS_LOW;
    SD_SendCommand(READ_SINGLE_BLOCK,address);  //Send CMD17 to return a single block;
    r1 = SD_GetR1(); // Get R1 response

    if(r1 > 0)
    {
        CS_HIGH
        return ( R1_ERROR | r1 );
    }

    if(r1 == 0)
    {
        timeout = 0;
        uint8_t r = SD_ReceiveByteSPI();
        while(r != 0xFE) //Start Block Token
        {
            r = SD_ReceiveByteSPI();
            timeout++;
            if(timeout > 254) 
            { 
                CS_HIGH;
                return (START_TOKEN_TIMEOUT | r1);
            }
        }

        // if start block token has been received and not timed out then begin reading in the data.
        if( (timeout < 0xFF) && (r == 0xFE) )
        {            
            for(uint16_t i = 0; i < DATA_BLOCK_LEN; i++)
                ds->byte[i] = SD_ReceiveByteSPI();

            for(uint8_t i = 0; i < 2; i++)
                ds->CRC[i] = SD_ReceiveByteSPI();
                
            SD_ReceiveByteSPI(); // clear any remaining characters in SD response.
        }
    }
    CS_HIGH;
    return READ_SUCCESS;
}
// END SD_ReadSingleDataBlock()



// Print the data in the array pointed to by *block.
void SD_PrintDataBlock(uint8_t *block)
{
    print_str("\n\n\r BLOCK OFFSET\t\t\t\t   HEX\t\t\t\t\t     ASCII\n\r");
    uint16_t block_offset = 0;
    int space = 0;
    for(uint16_t row=0; row<DATA_BLOCK_LEN/16; row++)
    {
        print_str("\n\r   ");
        if(block_offset<0x100)  { print_str("0x0"); print_hex(block_offset); }
        else if(block_offset<0x1000) { print_str("0x"); print_hex(block_offset); }
        
        print_str("\t ");
        space = 0;
        for(block_offset = row*16;block_offset<(row*16)+16;block_offset++)
        {
            //every 4 bytes print an extra space.
            if(space%4==0) 
                print_str(" ");

            print_str(" ");
            print_hex(block[block_offset]);
            space++;
        }
        
        print_str("\t\t");
        block_offset = block_offset-16;
        for(block_offset = row*16;block_offset<(row*16)+16;block_offset++)
        {
            if(block[block_offset]<32)
                USART_Transmit(' ');
            else if(block[block_offset]<128)
                USART_Transmit(block[block_offset]);
            else
                USART_Transmit('.');
        }
    }    
    print_str("\n\n\r");
}
// END SD_PrintDataBlock(uint8_t *block)



// Print multiple data blocks to screen.
void SD_PrintMultipleDataBlocks(uint32_t start_address, uint32_t numOfBlocks)
{
    Block ds;
    int timeout = 0;

    CS_LOW;
    SD_SendCommand(READ_MULTIPLE_BLOCK,start_address); // CMD18
    uint8_t r1 = SD_GetR1(); //get R1 response

    if(r1 > 0)
    {
        CS_HIGH
        print_str("\n\r>> ERROR:   READ_MULTIPLE_BLOCK (CMD18) in SD_PrintMultipleDataBlocks() returned error in R1 response.");
        SD_PrintR1(r1); //print R1 response if error was returned.
    }

    // if R1 response has no errors then read in / print the data block
    if(r1 == 0)
    {
        for (int i=0;i<numOfBlocks;i++)
        {
            // prior to returning each data block, a start block token must be sent.
            timeout = 0;
            while(SD_ReceiveByteSPI() != 0xFE) // wait for start block token.
            {
                timeout++;
                if(timeout > 511)
                {
                    print_str("\n\r>> ERROR:   Timeout while waiting for Start Block Token in SD_PrintMultipleDataBlocks().");
                    break;
                }
            }

            for(uint16_t k = 0; k < DATA_BLOCK_LEN; k++) ds.byte[k] = SD_ReceiveByteSPI(); // get data block
            for(uint8_t k = 0; k < 2; k++) ds.CRC[k] = SD_ReceiveByteSPI(); // get CRC response
            print_str("\n\n\r\t\t\t\t\tBLOCK: ");
            print_dec((start_address + (i*512))/512);
            SD_PrintDataBlock(ds.byte); // print data block
        }
        
        SD_SendCommand(STOP_TRANSMISSION,0); // stop data block tranmission 
        SD_ReceiveByteSPI(); //response doesn't matter
    }

    CS_HIGH;
}
// END sd_PrintMultiplDataBlocks()



// Prints the block number of all blocks between begin_block and end_block 
// that have any non-zero bytes, to assist in finding blocks that have data.
void SD_SearchNonZeroBlocks(uint32_t begin_block, uint32_t end_block)
{
    Block ds;
    print_str("\n\rSearching for non-zero blocks over range ");
    print_dec(begin_block);
    print_str(" to ");
    print_dec(end_block);
    int tab = 0; //used for printing format
    uint32_t Address = 0;
    //for(uint32_t block = begin_block; block<end_block+1; block++)
    for(uint32_t block = begin_block; block < end_block + 1; block++)
    {
        Address = block * 512;
        //ds = SD_ReadSingleDataBlock(Address);
        SD_ReadSingleDataBlock(Address, &ds);                
        
        for(int i = 0; i<512;i++)
        {
            if(ds.byte[i]!=0)
            {
                if(tab%5==0) print_str("\n\r");
                print_str("\t\t");print_dec(block);
                tab++;
                break;
            }
        }        
    }
    print_str("\n\rDone searching non-zero blocks.");
}
// END sd_SearchNonZeroDataBlocks()



// Writes data in the array pointed at by *dataBuffer to the block at 'address' 
uint16_t SD_WriteSingleDataBlock(uint32_t address, uint8_t *dataBuffer)
{
    uint8_t DataResponseToken;  // a data response token is returned from the SD card upon
                                // completion of sending an entire block of data.  It is
                                // used to indicate if the data was accepted or rejected.

    int timeout = 0; // used for timeout

    CS_LOW;    
    SD_SendCommand(WRITE_BLOCK,address);  //Send CMD24 to write a single data block at block address;
    uint8_t r1 = SD_GetR1(); // Get R1 response. If non-zero then error.

    //If R1 is non-zero or times out, then return with R1 response.
    if(r1 > 0)
    {
        CS_HIGH
        print_str("\n\r>> ERROR:   WRITE_BLOCK (CMD24) in SD_WriteSingleDataBlock() returned error in R1 response.");
        SD_PrintR1(r1);
        return (r1 | INVALID_DATA_RESPONSE);
    }

    //if R1 response is 0 then proceed with writing to data block
    if(r1 == 0)
    {
        SD_SendByteSPI(0xFE); // Send Start Block Token to SD card to signal initiating data transfer.

        // send data to write to SD card.
        for(uint16_t i = 0; i < DATA_BLOCK_LEN; i++)
            SD_SendByteSPI(dataBuffer[i]);

        // Send 16-bit CRC. CRC value does not matter if CRC is off (default).
        SD_SendByteSPI(0xFF);
        SD_SendByteSPI(0xFF);
        

        uint8_t dtMask = 0x1F; //Data Token Mask
        timeout = 0;

        do{ // loop until data response token received.
            
            DataResponseToken = SD_ReceiveByteSPI(); // get data token
            
            if(timeout++ > 0xFF)
            {
                CS_HIGH;
                print_str("\n\r>> ERROR:   Timeout while waiting for Data Response Token in SD_WriteSingleDataBlock().  Returning with INVALID_DATA_TOKEN\n\r");
                return (r1 | INVALID_DATA_RESPONSE);
            }  
             
        }while( ((dtMask&DataResponseToken)!=0x05) &&  // confirm valid data response token received.
                ((dtMask&DataResponseToken)!=0x0B) && 
                ((dtMask&DataResponseToken)!=0x0D) ); 
        
        // Return value corresponding to the data response token received.
        if((DataResponseToken&0x05)==5) // Data Accepted
        {
            int j = 0;
            
            // wait for SD card to not be busy, i.e. to finish writing data to the block.
            while(SD_ReceiveByteSPI() == 0) // DO (data out) line held low while card is busy writing data to block.
            {
                if(j > 512) { print_str("\n\r>> timeout waiting for card to not be busy.\n\r"); break; } 
                j++;
            };
            CS_HIGH
            return DATA_ACCEPTED;
        }

        else if((DataResponseToken&0x0B)==0x0B) // CRC Error
        {
            CS_HIGH;
            print_str("\n\r>> Data rejected Due to CRC Error\n\rData Response Token = ");
            print_hex(DataResponseToken);
            print_str("n\r");
            return CRC_ERROR;
        }

        else if((DataResponseToken&0x0D)==0x0D) // Write Error
        {
            CS_HIGH;
            print_str("\n\r>> Data rejected due to a Write Error\n\rData Response Token = ");
            print_hex(DataResponseToken);
            print_str("n\r");
            return WRITE_ERROR;
        }
    }
    return INVALID_DATA_RESPONSE; // successful write returns 0
}
//END SD_WriteSingleDataBlock()



// Writes data in the array pointed at by *dataBuffer to multiple
// blocks specified by 'nob' and beginning at 'address'. 
uint16_t SD_WriteMultipleDataBlocks(uint32_t address, uint8_t nob, uint8_t *dataBuffer)
{
    uint8_t DataResponseToken;  // a data response token is returned from the SD card upon
                                // completion of sending an entire block of data.  It is
                                // used to indicate if the data was accepted or rejected.
    
    uint16_t retVal = INVALID_DATA_RESPONSE; // used to hold return value. Initiated with 
                                             // INVALID_DATA_RESPONSE. Will be updated 
                                             // as data response tokens are received.

    int timeout = 0; // used for timeout
    
    CS_LOW;    
    SD_SendCommand(WRITE_MULTIPLE_BLOCK,address);  //Send CMD25 write data to multiple blocks.
    uint8_t r1 = SD_GetR1(); // Get R1 response.
    
    //If R1 is non-zero or times out, then return.
    if(r1 > 0)
    {
        CS_HIGH
        print_str("\n\r>> ERROR:   WRITE_BLOCK (CMD24) in SD_WriteMultipleDataBlocks() returned error in R1 response.");
        SD_PrintR1(r1);
        return (r1 | retVal); // retVal should be INVALID_DATA_RESPONSE here.
    }

    //if R1 response is 0 then proceed with writing dataBuffer to data block
    if(r1 == 0)
    {
        uint8_t dtMask = 0x1F; //Data Token Mask

        for(int k = 0; k < nob; k++)
        {
            retVal = INVALID_DATA_RESPONSE; //Reset retVal to INVALID_DATA_RESPONSE prior to sending each data block.
            SD_SendByteSPI(0xFC); // Send Start Block Token to SD card to signal initiating data transfer.

            // send data in dataBuffer to SD card.
            for(uint16_t i = 0; i < DATA_BLOCK_LEN; i++)
                SD_SendByteSPI(dataBuffer[i]);

            // Send 16-bit CRC. CRC value does not matter if CRC is off (default). Set using CRC_ON_OFF command.
            SD_SendByteSPI(0xFF); 
            SD_SendByteSPI(0xFF);




            timeout = 0;
            
            do{ // loop until data response token received.    
                DataResponseToken = SD_ReceiveByteSPI(); // get data token
                if(timeout++ > 0xFF)
                {
                    CS_HIGH;
                    print_str("\n\r>> ERROR:   Timeout while waiting for Data Response Token in SD_WriteMultipleDataBlocks().  Returning with INVALID_DATA_TOKEN\n\r");
                    return (r1 | retVal); // retVal should be INVALID_DATA_RESPONSE here.
                }  
                
            }while( ((dtMask&DataResponseToken)!=0x05) &&  // valid data response token received?
                    ((dtMask&DataResponseToken)!=0x0B) && 
                    ((dtMask&DataResponseToken)!=0x0D) ); 

            // Check which Data Response Token was received. Return value corresponding to the data response token received.
            if((DataResponseToken&0x05)==5) // Data Accepted
            {
                int j = 0;
                
                // wait for SD card to not be busy, i.e. to finish writing data to the block.
                while(SD_ReceiveByteSPI() == 0) // DO (data out) line held low while card is busy writing data to block.
                {
                    if(j > 512) { print_str("\n\r>> timeout waiting for card to not be busy.\n\r"); break; } 
                    j++;
                };
                retVal = DATA_ACCEPTED;
            }

            else if((DataResponseToken&0x0B)==0x0B) // CRC Error
            {
                print_str("\n\r>> Data rejected Due to CRC Error\n\rData Response Token = ");
                print_hex(DataResponseToken);
                print_str("n\r");
                retVal = CRC_ERROR;
                break;
            }

            else if((DataResponseToken&0x0D)==0x0D) // Write Error
            {
                print_str("\n\r>> Data rejected due to a Write Error\n\rData Response Token = ");
                print_hex(DataResponseToken);
                print_str("n\r");
                retVal = WRITE_ERROR;
                break;
            }
        }

        int j = 0;
        SD_SendByteSPI(0xFD); // send stop transmission token
        while(SD_ReceiveByteSPI() == 0) // DO (data out) line held low while card is busy writing data to block.
        {
            if(j > 512) { print_str("\n\r>> timeout waiting for card to not be busy.\n\r"); break; } 
            j++;
        }
    }
    CS_HIGH;
    return retVal; // successful write returns 0
}
//END SD_WriteMultipleDataBlocks()



// Prints the error code returned by SD_WriteSingleDataBlock().
void SD_PrintWriteError(uint16_t err)
{
    //print R1 portion of initiailzation response
    print_str("\n\r>> R1 Response returned by sd_WriteSingelDataBlock():");
    SD_PrintR1((uint8_t)(0x00FF&err));

    print_str("\n\r>> Data Response Errors:");
    
    switch(err&0x0700)
    {
        case(INVALID_DATA_RESPONSE):
            print_str("\n\r INVALID DATA RESPONSE");
            break;
        case(WRITE_ERROR):
            print_str("\n\r WRITE_ERROR");
            break;
        case(CRC_ERROR):
            print_str("\n\r CRC_ERROR");
            break;
        case(DATA_ACCEPTED):
            print_str("\n\r DATA ACCEPTED"); // Successful data write
            break;
        default:
            print_str("\n\r INCORRECT RESPONSE RETURNED");
    }
}
//END SD_PrintWriteError()



// Returns the number of well written blocks after a 
// multi-block write operation is performed.
uint32_t SD_NumberOfWellWrittenBlocks(void)
{
    uint32_t wwwb = 0; //well written blocks. initialized to zero
    uint8_t r1; // for R1 response.
    int count = 0; 


    CS_LOW; // Assert CS
    SD_SendCommand(APP_CMD,0); // signal that next command is an application command
    r1 = SD_GetR1();
    if(r1 > 0) 
    { 
        print_str("\n\r>> ERROR: error returned in R1 response to APP_CMD in SD_NumberOfWellWrittenBlocks(). Returning with invalid value");
        SD_PrintR1(r1);
        return 0;
    }

    SD_SendCommand(SEND_NUM_WR_BLOCKS,0);// number of well written write blocks
    r1 = SD_GetR1();
    if(r1 > 0) 
    { 
        print_str("\n\r>> ERROR: error returned in R1 response to SEND_NUM_WR_BLOCKS in SD_NumberOfWellWrittenBlocks(). Returning with invalid value");
        SD_PrintR1(r1);
        return 0;
    }
    //print_str("\n\r");SD_PrintR1(SD_GetR1());
    
    while(SD_ReceiveByteSPI() != 0xFE) // start block token
    {
        if(count++ > 0xFE) 
        { 
            print_str("\n\r>> ERROR: timeout while waiting for start token in SD_NumberOfWellWrittenBlocks(). Returning with invalid value"); 
            return 0;
        }

    }

    // Get the number of well written blocks (32-bit)
    wwwb = SD_ReceiveByteSPI();
    wwwb <<= 8;
    wwwb |= SD_ReceiveByteSPI();
    wwwb <<= 8;
    wwwb |= SD_ReceiveByteSPI();
    wwwb <<= 8;
    wwwb |= SD_ReceiveByteSPI();

    // CRC bytes
    SD_ReceiveByteSPI();
    SD_ReceiveByteSPI();

    CS_HIGH; // Deassert CS

    return wwwb;
}
//END SD_NumberOfWellWrittenBlocks()



// Erases blocks from start_address to end_address (inclusive)
uint16_t SD_EraseBlocks(uint32_t start_address, uint32_t end_address)
{
    uint8_t r1 = 0;
    
    // set start address for erase block
    CS_LOW;
    SD_SendCommand(ERASE_WR_BLK_START_ADDR,start_address);
    r1 = SD_GetR1();
    CS_HIGH;

    //If R1 is non-zero or times out, then return with R1 response.
    if(r1 > 0)
    {
        print_str("\n\r>> ERROR:   ERASE_WR_BLOCK_START_ADDR (CMD32) in SD_EraseBlocks() returned error in R1 response.");
        SD_PrintR1(r1);
        return (r1 | ERROR_ERASE_START_ADDR);
    }
    
    // set end address for erase block
    CS_LOW;
    SD_SendCommand(ERASE_WR_BLK_END_ADDR,end_address);
    r1 = SD_GetR1();
    CS_HIGH;
    //If R1 is non-zero or times out, then return with R1 response.
    if(r1 > 0)
    {
        print_str("\n\r>> ERROR:   ERASE_WR_BLOCK_END_ADDR (CMD33) in SD_EraseBlocks() returned error in R1 response.");
        SD_PrintR1(r1);
        return (r1 | ERROR_ERASE_END_ADDR);
    }

    // erase all blocks in range of start address to end address
    CS_LOW;
    SD_SendCommand(ERASE,0);
    r1 = SD_GetR1();
    
    //If R1 is non-zero or times out, then return with R1 response.
    if(r1 > 0)
    {
        CS_HIGH;
        print_str("\n\r>> ERROR:   ERASE (CMD38) in SD_EraseBlocks() returned error in R1 response.");
        SD_PrintR1(r1);
        return (r1 | ERROR_ERASE);
    }

    uint16_t timeout = 0; 

    // DO (Data Out) line will be held low by card while it completes erase of data blocks.
    // once it is complete the DO line will be any non-zero value to signal it is out of the busy state.
    while(SD_ReceiveByteSPI() == 0)
    {
        if(timeout++ > 0xFFFE) 
        {
            print_str("\n\r>> ERROR:    Timeout while waiting for card to exit the busy state during block erase in SD_EraseBlockss(). Returning with error(s).\n\r");
            return (r1 | ERROR_BUSY);
        }
    }
    CS_HIGH;
    return ERASE_SUCCESSFUL;
}
// END SD_EraseBlocks()



// Prints the error code returned by SD_Eraseblocks()
void SD_PrintEraseBlockError(uint16_t err)
{
    //print R1 portion of initiailzation response
    if(SD_MSG > 1) print_str("\n\r>> INFO:    R1 Response returned by SD_EraseBlocks():");
    SD_PrintR1((uint8_t)(0x00FF&err));

    if(SD_MSG > 1) print_str("\n\r>> INFO:    SD_EraseBlocks():");
    
    switch(err&0xFF00)
    {
        case(ERASE_SUCCESSFUL):
            print_str("\n\r\t    ERASE SUCCESSFUL");
            break;
        case(ERROR_ERASE_START_ADDR):
            print_str("\n\r\t    ERROR ERASE START ADDRESS");
            break;
        case(ERROR_ERASE_END_ADDR):
            print_str("\n\r\t    ERROR ERASE END ADDRESS");
            break;
        case(ERROR_ERASE):
            print_str("\n\r\t    ERROR ERASE"); // Successful data write
            break;
        case(ERROR_BUSY):
            print_str("\n\r\t    ERROR BUSY"); // Successful data write
            break;
        default:
            print_str("\n\r\t    INVALID ERROR RESPONSE");
    }
}
//END SD_PrintWriteError()