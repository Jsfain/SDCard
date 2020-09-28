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
#include "../includes/usart.h"
#include "../includes/spi.h"
#include "../includes/prints.h"
#include "../includes/sd_spi_base.h"
#include "../includes/sd_spi_data_access.h"



// Read single block at address from SD card into array.
uint16_t SD_ReadSingleBlock(uint32_t blockAddress, Block *bl)
{
    uint8_t timeout = 0;
    uint8_t r1;

    CS_LOW;
    SD_SendCommand(READ_SINGLE_BLOCK, blockAddress); //CMD17
    r1 = SD_GetR1();

    if(r1 > 0)
    {
        CS_HIGH;
        return ( R1_ERROR | r1 );
    }

    if(r1 == 0)
    {
        timeout = 0;
        uint8_t sbt = SD_ReceiveByteSPI(); 
        while(sbt != 0xFE) // wait for Start Block Token
        {
            sbt = SD_ReceiveByteSPI();
            timeout++;
            if(timeout > 0xFE) 
            { 
                CS_HIGH;
                return ( START_TOKEN_TIMEOUT | r1 );
            }
        }

        // start block token has been received         
        for(uint16_t i = 0; i < BLOCK_LEN; i++)
            bl->byte[i] = SD_ReceiveByteSPI();

        for(uint8_t i = 0; i < 2; i++)
            bl->CRC[i] = SD_ReceiveByteSPI();
            
        SD_ReceiveByteSPI(); // clear any remaining bytes in SPDR
    }
    CS_HIGH;
    return ( READ_SUCCESS | r1 );
}
// END SD_ReadSingleBlock()



// Print columnized address offset | HEX | ASCII values in *byte array
void SD_PrintBlock(uint8_t *byte)
{
    print_str("\n\n\r BLOCK OFFSET\t\t\t\t   HEX\t\t\t\t\t     ASCII\n\r");
    uint16_t offset = 0;
    uint16_t space = 0;
    for( uint16_t row = 0; row < BLOCK_LEN/16; row++ )
    {
        print_str("\n\r   ");
        if(offset<0x100){print_str("0x0"); print_hex(offset);}
        else if(offset<0x1000){print_str("0x"); print_hex(offset);}
        
        print_str("\t ");
        space = 0;
        for( offset = row * 16; offset < (row*16)+16; offset++ )
        {
            //every 4 bytes print an extra space.
            if(space%4==0) 
                print_str(" ");

            print_str(" ");
            print_hex(byte[offset]);
            space++;
        }
        
        print_str("\t\t");
        offset = offset-16;
        for( offset = row * 16; offset < (row*16) + 16; offset++ )
        {
            if( byte[offset] < 32 ) { USART_Transmit( ' ' ); }
            else if( byte[offset] < 128 ) { USART_Transmit( byte[offset] ); }
            else USART_Transmit('.');
        }
    }    
    print_str("\n\n\r");
}
// END SD_PrintBlock(uint8_t *byte)



// Print multiple data blocks to screen.
uint16_t SD_PrintMultipleBlocks(
                uint32_t startBlockAddress,
                uint32_t numberOfBlocks)
{
    Block bl;
    uint16_t timeout = 0;
    uint8_t r1;

    CS_LOW;
    SD_SendCommand(READ_MULTIPLE_BLOCK, startBlockAddress); // CMD18
    r1 = SD_GetR1();
    print_str("\n\rprinting R1 = ");SD_PrintR1(r1);
    if(r1 > 0)
    {
        CS_HIGH
        return ( R1_ERROR | r1 );
    }

    if(r1 == 0)
    {
        for ( int i = 0; i < numberOfBlocks; i++ )
        {
            timeout = 0;
            while( SD_ReceiveByteSPI() != 0xFE ) // wait for start block token.
            {
                timeout++;
                if(timeout > 0x500) return (START_TOKEN_TIMEOUT | r1);
            }

            for(uint16_t k = 0; k < BLOCK_LEN; k++) 
                bl.byte[k] = SD_ReceiveByteSPI();

            for(uint8_t k = 0; k < 2; k++) 
                bl.CRC[k] = SD_ReceiveByteSPI();

            
            print_str("\n\n\r\t\t\t\t\tBLOCK: ");
            print_dec( (startBlockAddress + (i * BLOCK_LEN)) / BLOCK_LEN );
            SD_PrintBlock(bl.byte);
        }
        
        SD_SendCommand(STOP_TRANSMISSION,0);
        // response should be R1b, which is R1 and number of busy signal bytes.
        // may include code to hanlde this later, but right now just emptying
        // the SPDR of the response.
        SD_ReceiveByteSPI(); 
    }

    CS_HIGH;
    return READ_SUCCESS;
}
// END sd_PrintMultiplDataBlocks()



// Writes data in array pointed at by *data to the block at 'address'
uint16_t SD_WriteSingleDataBlock(uint32_t blockAddress, uint8_t *data)
{
    uint8_t  r1;
    CS_LOW;    
    SD_SendCommand(WRITE_BLOCK,blockAddress); // CMD24
    r1 = SD_GetR1();

    if(r1 > 0)
    {
        CS_HIGH
        return ( R1_ERROR | r1 );
    }

    if(r1 == 0)
    {
        SD_SendByteSPI(0xFE); // Send Start Block Token initiates data transfer

        // send data to write to SD card.
        for(uint16_t i = 0; i < BLOCK_LEN; i++) SD_SendByteSPI(data[i]);

        // Send 16-bit CRC. CRC is off by default so these do not matter.
        SD_SendByteSPI(0xFF);
        SD_SendByteSPI(0xFF);
        
        uint8_t dataResponseToken;
        uint8_t dataTokenMask = 0x1F;
        uint16_t timeout = 0;

        do{ // wait for valid data response token
            
            dataResponseToken = SD_ReceiveByteSPI();
            
            if(timeout++ > 0xFE)
            {
                CS_HIGH;
                return ( DATA_RESPONSE_TIMEOUT | r1 );
            }  
             
        }while( ( (dataTokenMask & dataResponseToken) != 0x05) && // DATA_ACCEPTED
                ( (dataTokenMask & dataResponseToken) != 0x0B) && // CRC_ERROR
                ( (dataTokenMask & dataResponseToken) != 0x0D) ); // WRITE_ERROR
        
        if( ( dataResponseToken & 0x05 ) == 5) // DATA_ACCEPTED
        {
            timeout = 0;
            
            // Wait for SD card to finish writing data to the block.
            // Data out line held low while card is busy writing to block.
            while(SD_ReceiveByteSPI() == 0) 
            {
                if(timeout++ > 0x1FF) 
                {
                    CS_HIGH;
                    return (CARD_BUSY_TIMEOUT | r1);
                }
            };
            CS_HIGH;
            return (DATA_ACCEPTED_TOKEN | r1);
        }

        else if((dataResponseToken & 0x0B ) == 0x0B ) // CRC Error
        {
            CS_HIGH;
            return (CRC_ERROR_TOKEN | r1);
        }

        else if((dataResponseToken&0x0D)==0x0D) // Write Error
        {
            CS_HIGH;
            return (WRITE_ERROR_TOKEN | r1);
        }
    }
    return (INVALID_DATA_RESPONSE | r1) ;
}
//END SD_WriteSingleDataBlock()



// Writes data in the array pointed at by *data to multiple blocks
// specified by 'numberOfBlocks' and starting at 'address'.
uint16_t SD_WriteMultipleBlocks(
                uint32_t blockAddress, 
                uint32_t numberOfBlocks, 
                uint8_t *data)
{
    uint8_t dataResponseToken;
    uint16_t returnToken;
    uint16_t timeout = 0;

    CS_LOW;    
    SD_SendCommand(WRITE_MULTIPLE_BLOCK,blockAddress);  //CMD25
    uint8_t r1 = SD_GetR1();
    
    if(r1 > 0)
    {
        CS_HIGH
        return (R1_ERROR | r1);
    }

    if(r1 == 0)
    {
        uint8_t dataTokenMask = 0x1F;

        for(int k = 0; k < numberOfBlocks; k++)
        {
            SD_SendByteSPI(0xFC); // Start Block Token initiates data transfer

            // send data in 'data' to SD card.
            for(uint16_t i = 0; i < BLOCK_LEN; i++)
                SD_SendByteSPI(data[i]);

            // Send 16-bit CRC. CRC is off by default so these do not matter.
            SD_SendByteSPI(0xFF);
            SD_SendByteSPI(0xFF);

            uint16_t timeout = 0;
            
            do{ // wait for valid data response token

                dataResponseToken = SD_ReceiveByteSPI();
                if(timeout++ > 0xFF)
                {
                    CS_HIGH;
                    return (DATA_RESPONSE_TIMEOUT | r1);
                }  
                
            }while( ((dataTokenMask & dataResponseToken) != 0x05) &&
                    ((dataTokenMask & dataResponseToken) != 0x0B) && 
                    ((dataTokenMask & dataResponseToken) != 0x0D) ); 


            if( (dataResponseToken & 0x05) == 5 ) // DATA ACCEPTED
            {
                timeout = 0;
                
                // Wait for SD card to finish writing data to the block.
                // Data out line held low while card is busy writing to block.
                while(SD_ReceiveByteSPI() == 0)
                {
                    if(timeout++ > 511) return (CARD_BUSY_TIMEOUT | r1); 
                };
                returnToken = DATA_ACCEPTED_TOKEN;
            }

            else if( (dataResponseToken & 0x0B) == 0x0B ) // CRC Error
            {
                returnToken = CRC_ERROR_TOKEN;
                break;
            }

            else if( (dataResponseToken & 0x0D) == 0x0D ) // Write Error
            {
                returnToken = WRITE_ERROR_TOKEN;
                break;
            }
        }

        timeout = 0;
        SD_SendByteSPI(0xFD); // Stop Transmission Token
        while(SD_ReceiveByteSPI() == 0)
        {
            if(timeout++ > 511) 
            {
                CS_HIGH;
                return (CARD_BUSY_TIMEOUT | r1);
            }
        }
    }
    CS_HIGH;
    return returnToken; // successful write returns 0
}
//END SD_WriteMultipleDataBlocks()




// Returns the number of well written blocks after a multi-block
// write operation is performed. Use SD_PrintReadError(err) 
uint16_t SD_NumberOfWellWrittenBlocks(uint32_t *wellWrittenBlocks)
{
    uint8_t r1;
    uint8_t timeout = 0; 

    CS_LOW;
    SD_SendCommand(APP_CMD,0); // next command is ACM
    r1 = SD_GetR1();
    if(r1 > 0) 
    {   
        CS_HIGH;
        return ( R1_ERROR | r1);
    }

    SD_SendCommand(SEND_NUM_WR_BLOCKS,0);
    r1 = SD_GetR1();
    if(r1 > 0)
    {
        CS_HIGH;
        return ( R1_ERROR | r1);
    }
    
    while(SD_ReceiveByteSPI() != 0xFE) // start block token
    {
        if(timeout++ > 0xFE) 
        {
            CS_HIGH;
            return ( START_TOKEN_TIMEOUT | r1 );
        }
    }

    // Get the number of well written blocks (32-bit)
    *wellWrittenBlocks = SD_ReceiveByteSPI();
    *wellWrittenBlocks <<= 8;
    *wellWrittenBlocks |= SD_ReceiveByteSPI();
    *wellWrittenBlocks <<= 8;
    *wellWrittenBlocks |= SD_ReceiveByteSPI();
    *wellWrittenBlocks <<= 8;
    *wellWrittenBlocks |= SD_ReceiveByteSPI();

    // CRC bytes
    SD_ReceiveByteSPI();
    SD_ReceiveByteSPI();

    CS_HIGH;

    return READ_SUCCESS;
}
//END SD_NumberOfWellWrittenBlocks()



// Erases blocks from start_address to end_address (inclusive)
uint16_t SD_EraseBlocks(uint32_t startBlockAddress, uint32_t endBlockAddress)
{
    uint8_t r1 = 0;
    
    // set start address for erase block
    CS_LOW;
    SD_SendCommand(ERASE_WR_BLK_START_ADDR,startBlockAddress);
    r1 = SD_GetR1();
    CS_HIGH;
    if(r1 > 0) return (SET_ERASE_START_ADDR_ERROR | R1_ERROR | r1);
    
    // set end address for erase block
    CS_LOW;
    SD_SendCommand(ERASE_WR_BLK_END_ADDR,endBlockAddress);
    r1 = SD_GetR1();
    CS_HIGH;
    if(r1 > 0) return (SET_ERASE_END_ADDR_ERROR | R1_ERROR | r1);

    // erase all blocks in range
    CS_LOW;
    SD_SendCommand(ERASE,0);
    r1 = SD_GetR1();
    if(r1 > 0)
    {
        CS_HIGH;
        return ( ERROR_ERASE | R1_ERROR | r1 );
    }

    uint16_t timeout = 0; 

    // wait for card not to finish erasing blocks.
    while(SD_ReceiveByteSPI() == 0)
    {
        if(timeout++ > 0xFFFE) return (ERASE_BUSY_TIMEOUT | r1);
    }
    CS_HIGH;
    return ERASE_SUCCESSFUL;
}
// END SD_EraseBlocks()



// Prints the error code returned by a write function.
void SD_PrintWriteError(uint16_t err)
{
    switch(err&0xFF00)
    {
        case(DATA_ACCEPTED_TOKEN):
            print_str("\n\r DATA ACCEPTED");
            break;
        case(CRC_ERROR_TOKEN):
            print_str("\n\r CRC_ERROR");
            break;
        case(WRITE_ERROR_TOKEN):
            print_str("\n\r WRITE_ERROR");
            break;
        case(INVALID_DATA_RESPONSE):
            print_str("\n\r INVALID_DATA_RESPONSE"); // Successful data write
            break;
        case(DATA_RESPONSE_TIMEOUT):
            print_str("\n\r DATA_RESPONSE_TIMEOUT");
            break;
        case(CARD_BUSY_TIMEOUT):
            print_str("\n\r CARD_BUSY_TIMEOUT");
            break;
        case(R1_ERROR):
            print_str("\n\r R1_ERROR"); // Successful data write
            break;
        default:
            print_str("\n\r UNKNOWN RESPONSE");
    }
}
//END SD_PrintWriteError()



// Prints the error code returned by a read function.
void SD_PrintReadError(uint16_t err)
{
    switch(err&0xFF00)
    {
        case(R1_ERROR):
            print_str("\n\r R1 ERROR");
            break;
        case(READ_SUCCESS):
            print_str("\n\r READ SUCCESS");
            break;
        case(START_TOKEN_TIMEOUT):
            print_str("\n\r START TOKEN TIMEOUT");
            break;
        default:
            print_str("\n\r UNKNOWN RESPONSE");
    }
}
//END SD_PrintWriteError()



// Prints the error code returned by SD_Eraseblocks()
void SD_PrintEraseError(uint16_t err)
{
    switch(err&0xFF00)
    {
        case(ERASE_SUCCESSFUL):
            print_str("\n\r ERASE SUCCESSFUL");
            break;
        case(SET_ERASE_START_ADDR_ERROR):
            print_str("\n\r SET ERASE START ADDR ERROR");
            break;
        case(SET_ERASE_END_ADDR_ERROR):
            print_str("\n\r SET ERASE END ADDR ERROR");
            break;
        case(ERROR_ERASE):
            print_str("\n\r ERROR ERASE");
            break;
        case(ERASE_BUSY_TIMEOUT):
            print_str("\n\r ERASE_BUSY_TIMEOUT");
            break;
        default:
            print_str("\n\r UNKNOWN RESPONSE");
    }
}
//END SD_PrintWriteError()
