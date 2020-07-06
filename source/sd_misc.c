/*********************************************************************************************
 * Author: Joshua Fain
 * Date:   7/5/2020
 * 
 * File: SD_MISCH.C 
 * 
 * Requires: SD_MISC.H - header for functions defined here
 *           SD_SPI.H - needed for direct interaction with the SD Card.
 *           SPI.H     - needed for sending commands to, and receiving
 *                       responses from, SD Card
 *           PRINTS.H  - needed for various print functions used here
 *           STDINT.H  - needed data types used here
 *           AVR/IO.H  - needed for I/O related AVR variables.
 * 
 * Target: ATmega 1280
 *
 * 
 * Description: 
 * Defines functions declared in SD_MISC.H. These functions defined here may be 
 * useful, but are not necessary, for interaction with the SD Card, unlike those
 * in SD_SPI.C. They are intended to provide some calculations, print, and 
 * get SD Card related functions.
 * 
 * 
 * Functions:
 * 1) uint32_t      sd_GetMemoryCapacity(void)
 * 2) DataSector    sd_ReadSingleDataSector(uint32_t address)
 * 3) void          sd_PrintSector(uint8_t *sector)
 * 4) void          sd_PrintMultipleDataBlocks(uint32_t start_address, uint32_t numOfBlocks)
 * 5) void          sd_SearchNonZeroSectors(uint32_t begin_sector, uint32_t end_sector)
 * 6) uint16_t      sd_WriteSingleDataBlock(uint32_t address, uint8_t *dataBuffer)
 * 7) void          sd_PrintWriteError(uint16_t err)
 * 8) uint16_t      sd_EraseSector(uint32_t start_address, uint16_t end_address)
 * 9) void          sd_PrintEraseSectorError(uint16_t err)
 * 
 * Notes:
 * Other functions will be included as needed. 
 * ******************************************************************************************/



#include <stdint.h>
#include <avr/io.h>
#include "../includes/usart.h"
#include "../includes/prints.h"
#include "../includes/sd_spi.h"
#include "../includes/sd_misc.h"



/******************************************************************************
 * Function:    sd_getMemoryCapacity(void) 
 * Description: Calculates and returns the memory capacity (in BYTES) of the 
 *              SD card based on values of the CSD register parameters C_SIZE,
 *              READ_BL_LEN, and C_SIZE_MULT.
 * Argument(s): VOID
 * Returns:     Integer value of the memory capcity in bytes if successful.
 *              1 if unsuccessful.
 * Notes:       The function reads in the bytes of the CSD register and 
 *              performs checks on the returned values of the CSD where 
 *              possible, not just those used for the calculation.  
 *              See SD Card Physical Layer Specification for specifics
 *              regarding the calculation.
******************************************************************************/
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
    CS_ASSERT;
    sd_SendCommand(SEND_CSD,0); //CMD9 - Request CSD Register from SD card.
    R1 = sd_getR1(); // Get R1 response

    //If R1 is non-zero or times out, then return without getting rest of CSD.
    if(R1 > 0)
    {
        CS_DEASSERT
        print_str("\n\r>> ERROR:   SEND_CSD (CMD9) in sd_getMemoryCapacity() returned error in R1 response.");
        return 1;
    }

    
    // ***** Get rest of the response bytes which are the CSD Register and CRC.
    
    
    attempt = 0;
    do{ // get CSD_STRUCTURE (Not used for memory calculation)
        if((sd_Response()>>6) <= 2) break; //check for valid CSD_STRUCTURE returned
        if(attempt++ >= 0xFF){ CS_DEASSERT; return 1;} // Timeout returning valid CSD_STRUCTURE 
    }while(1);
    
    attempt = 0;
    do{ // get TAAC (Not used for memory calculation)
        if(!(sd_Response()>>7)) break; // check for valid TAAC (bit 7 is rsvd should = 0)
        if(attempt++ >= 0xFF) { CS_DEASSERT; return 1;} //Timeout returning valid TAAC
    }while(1);

    sd_Response();  // get NSAC of CSD. Any value could be valid. (Not used for memory calculation)

    attempt = 0;
    do{ // get TRAN_SPEED. (Not used for memory calculation)
        resp = sd_Response();  
        if((resp == 0x32) || (resp == 0x5A)) break; //TRAN_SPEED must be 0x32 or 0x5A.
        if(attempt++ >= 0xFF) { CS_DEASSERT; return 1;} //Timeout returning valid TRAN_SPEED
    }while(1);


    //Next 2 bytes include 12 bit CCC and 4 bit READ_BL_LENGTH.  
    //CCC is of the form 01_110110101 and is NOT used for memory calculation
    //READ_BL_LEN is needed to calculate memory capacity and must by 9, 10, or 11.
    uint8_t flag = 1;
    while(flag == 1)
    {
        if((resp = sd_Response())&0x7B) //CCC 8 most significant bits must be of the form 01_11011;
        {
            if(((resp = sd_Response())>>4) == 0x05) //CCC least sig. 4 bits and 4 bit READ_BL_LEN.
            {
                READ_BL_LEN = resp&0b00001111;
                if ((READ_BL_LEN < 9) || (READ_BL_LEN > 11)) { CS_DEASSERT; return 1; }
                flag = 0;
            }
        }
        if(attempt++ >= 0xFF) { CS_DEASSERT; return 1;}
    }


    //This section gets the remaining bits of the CSD.
    //C_SIZE and C_SIZE_MULT are needed for memory capacity calculation.
    flag = 1;
    attempt = 0;
    while(flag == 1)
    {
        if((resp = sd_Response())&0xF3) //MASK of 0xF3 is based on the following. X is 1 or 0.
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
            C_SIZE |= sd_Response(); //get next 8 bits of C_SIZE
            C_SIZE <<= 2;        
            C_SIZE |= ((sd_Response())>>6);
            
            //get C_SIZE_MULT
            C_SIZE_MULT = ((sd_Response()&0x03)<<1);
            C_SIZE_MULT |= (sd_Response()>>7);
            
            flag = 0;
        }
        if(attempt++ >= 0xFF) { CS_DEASSERT; return 1; }
    }

    CS_DEASSERT;

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



/******************************************************************************
 * Function:    sd_ReadSingleDataSector(uint32_t address)
 * Description: Reads in a single data sector from the SD card at address
 *              specified in the argument and stores in a DataSector struct
 *              variable that includes the data bytes of the sector, errors 
 *              returned, and CRC returned. 
 * Argument(s): Address of data sector to read.
 * Returns:     DataSector struct. See SD_MISC.H for details.
 * Notes:       The length of the data sector is specified by the 
 *              DATA_BLOCK_LEN defined in SD_SPI.H. This should be 512-bytes.
******************************************************************************/
DataSector sd_ReadSingleDataBlock(uint32_t address)
{
    DataSector ds;

    uint8_t attempt = 0;
    print_str("\n\rReading Address: ");
    print_dec(address);
    CS_ASSERT;
    sd_SendCommand(READ_SINGLE_BLOCK,address);  //Send CMD17 to return a single block;
    ds.R1 = sd_getR1(); // Get R1 response

    //If R1 is non-zero or times out, then return without getting rest of block.
    if(ds.R1 > 0)
    {
        CS_DEASSERT
        print_str("\n\r>> ERROR:   READ_SINGLE_BLOCK (CMD17) in sd_ReadSingleDataSector() returned error in R1 response.");
        sd_printR1(ds.R1);
        ds.ERROR = 1;
        return ds;
    }

    if(ds.R1 == 0)
    {
        attempt = 0;
        uint8_t r = sd_Response();
        while(r != 0xFE)//Start Block Token
        {
            r = sd_Response();
            attempt++;
            if(attempt > 512)
            {
                print_str("\n\r>> ERROR:   Timeout while waiting for Start Block Token in sd_ReadSingleDataSector().");
                ds.ERROR = 1;
                return ds;
            }
        }

        // if start block token has been received and not timed out then begin reading in the data.
        if((attempt < 0xFF) && (r== 0xFE))
        {            
            for(uint16_t i = 0; i < DATA_BLOCK_LEN; i++)
                ds.data[i] = sd_Response();

            for(uint8_t i = 0; i < 2; i++)
                ds.CRC[i] = sd_Response();
                
            sd_Response(); // clear any remaining characters in SD response.
        }
    }
    CS_DEASSERT;
    return ds;
}
//END sd_ReadSingleDataSector()



/******************************************************************************
 * Function:    sd_PrintSector(uint8_t *sector)
 * Description: Prints the contents of the data sector array passed in as the
 *              arguement. Prints the hexadecimal value of the data in 1 
 *              section and the corresponding ASCII characters (if valid).
 *              Each data row is prefixed with a sector offset value that
 *              corresponds to the address offset of the data in the first 
 *              column of that row.
 * Argument(s): 8-bit unsigned integer array of length DATA_BLOCK_LEN (defined
 *              in SD_SPI.H and should = 512).  
 * Returns:     VOID
 * Notes:       If the data does not correspond to a printable ASCII character 
 *              then the function will print an empty character ' ' if data
 *              is one of the control characters (i.e. <32) and a '.' 
 *              character if the data value is > 128.
******************************************************************************/
void sd_PrintSector(uint8_t *sector)
{
    print_str("\n\n\rSECTOR OFFSET\t\t\t\t   HEX\t\t\t\t\t     ASCII\n\r");
    uint16_t sector_offset = 0;
    int space = 0;
    for(uint16_t row=0; row<DATA_BLOCK_LEN/16; row++)
    {
        print_str("\n\r   ");
        if(sector_offset<0x100)  { print_str("0x0"); print_hex(sector_offset); }
        else if(sector_offset<0x1000) { print_str("0x"); print_hex(sector_offset); }
        
        print_str("\t ");
        space = 0;
        for(sector_offset = row*16;sector_offset<(row*16)+16;sector_offset++)
        {
            //every 4 bytes print an extra space.
            if(space%4==0) 
                print_str(" ");

            print_str(" ");
            print_hex(sector[sector_offset]);
            space++;
        }
        
        print_str("\t\t");
        sector_offset = sector_offset-16;
        for(sector_offset = row*16;sector_offset<(row*16)+16;sector_offset++)
        {
            if(sector[sector_offset]<32)
                USART_Transmit(' ');
            else if(sector[sector_offset]<128)
                USART_Transmit(sector[sector_offset]);
            else
                USART_Transmit('.');
        }
    }    
    print_str("\n\n\r");
}
// END sd_PrintSector(uint8_t *sector)



/****************************************************************************************
 * Function:    sd_PrintMultipleDataBlocks(uint32_t start_address, uint32_t numOfBlocks)
 * Description: Prints multiple, consecutive data blocks using READ_MULTIPLE_BLOCK
 *              command and sd_PrintSector(). The range of data blocks to be printed 
 *              begin at start_address and ending at start_address + (numOfBlocks - 1).
 * Argument(s): start address and number of blocks to read.
 * Returns:     VOID
 * Notes:       
****************************************************************************************/
void sd_PrintMultipleDataBlocks(uint32_t start_address, uint32_t numOfBlocks)
{
    DataSector ds;
    int attempt = 0;

    CS_ASSERT;
    sd_SendCommand(READ_MULTIPLE_BLOCK,start_address); 
    ds.R1 = sd_getR1(); //get R1 response

    if(ds.R1 > 0)
    {
        CS_DEASSERT
        print_str("\n\r>> ERROR:   READ_MULTIPLE_BLOCK (CMD18) in sd_PrintMultipleDataBlocks() returned error in R1 response.");
        sd_printR1(ds.R1); //print R1 response if error was returned.
    }

    // if R1 response has no errors then read in / print the data block
    if(ds.R1 == 0)
    {
        for (int i=0;i<numOfBlocks;i++)
        {
            // prior to returning each data block, a start block token must be sent.
            attempt = 0;
            while(sd_Response() != 0xFE) // wait for start block token.
            {
                attempt++;
                if(attempt > 511)
                {
                    print_str("\n\r>> ERROR:   Timeout while waiting for Start Block Token in sd_ReadSingleDataSector().");
                    ds.ERROR = 1;
                    break;
                }
            }

            for(uint16_t k = 0; k < DATA_BLOCK_LEN; k++) ds.data[k] = sd_Response(); // get data block
            for(uint8_t k = 0; k < 2; k++) ds.CRC[k] = sd_Response(); // get CRC response
            print_str("\n\n\rSECTOR ADDRESS: ");
            print_dec(start_address + (i*512));
            sd_PrintSector(ds.data); // print data block
        }
        
        sd_SendCommand(STOP_TRANSMISSION,0); // stop data block tranmission 
        sd_Response(); //response doesn't matter
    }

    CS_DEASSERT;
}
// END sd_PrintMultiplDataBlocks()



/***********************************************************************************
 * Function:    sd_SearchNonZeroSectors(uint32_t begin_sector, uint32_t end_sector)
 * Description: Searches between a specified range of sectors for any sectors that
 *              have non-zero values and prints those sector numbers to screen. 
 * Argument(s): 2 32-bit arguments that specify the beginning and ending sector in 
 *              the search.
 * Returns:     VOID
 * Notes:       
***********************************************************************************/
void sd_SearchNonZeroSectors(uint32_t begin_sector, uint32_t end_sector)
{
    DataSector ds;
    print_str("\n\rSearching for non-zero sectors over sector range ");
    print_dec(begin_sector);
    print_str(" to ");
    print_dec(end_sector);
    int tab = 0; //used for printing format
    uint32_t Address = 0;
    //for(uint32_t sector = begin_sector; sector<end_sector+1; sector++)
    for(uint32_t sector = begin_sector; sector < end_sector + 1; sector++)
    {
        Address = sector * 512;
        ds = sd_ReadSingleDataBlock(Address);                
        
        for(int i = 0; i<512;i++)
        {
            if(ds.data[i]!=0)
            {
                if(tab%5==0) print_str("\n\r");
                print_str("\t\t");print_dec(sector);
                tab++;
                break;
            }
        }        
    }
    print_str("\n\rDone searching non-zero sectors.");
}
// END sd_SearchNonZeroSectors()



/****************************************************************************************
 * Function:    sd_WriteSingleDataBlock(uint32_t address, uint8_t *dataBuffer)
 * Description: Writes the data in the dataBuffer to the sector specified by address.
 * Argument(s): uint32_t address:       address of data sector to write to.
 *              uint8_t  *dataBuffer:   pointer to array (length DATA_BLOCK_LEN) of data
 *                                      bytes that will be written to the sector 
 *                                      specified by address.
 * Returns:     Value corresponding to one of the Data Response Codes and R1 response. 
 * Notes:       1) Byte returned by SD Card for the Data Response Token is of the form
 *                 xxx0SSS1, where:
 *                                 xxx = don't care 
 *                                 SSS = 010: Data Accepted
 *                                 SSS = 101: Data rejected due to a CRC Error.
 *                                 SSS = 101: Data rejected due to a Write Error.
 * 
 *              2) If returned value indicates a write error occurred then the SEND_STATUS 
 *                 command should be sent in order to get the cause of the write error.  
 *              3) The data buffer length should be the value of DATA_BLOCK_LEN set in 
 *                 SD_SPI.H. This value should always be 512 (bytes).
*******************************************************************************************/
uint16_t sd_WriteSingleDataBlock(uint32_t address, uint8_t *dataBuffer)
{
    uint8_t DataResponseToken;  // data response token is returned from SD card
                                // once all the data block has been sent.

    int attempt = 0; // used for timeout

    CS_ASSERT;    
    sd_SendCommand(WRITE_BLOCK,address);  //Send CMD24 to write a single data block at sector address;
    uint8_t R1 = sd_getR1(); // Get R1 response. If non-zero then error.

    //If R1 is non-zero or times out, then return with R1 response.
    if(R1 > 0)
    {
        CS_DEASSERT
        print_str("\n\r>> ERROR:   WRITE_BLOCK (CMD24) in sd_WriteSingleDataSector() returned error in R1 response.");
        sd_printR1(R1);
        return (R1 | INVALID_DATA_RESPONSE);
    }

    //if R1 response is 0 then proceed with writing to data block
    if(R1 == 0)
    {
        sd_SendDataByte(0xFE); // Send Start Block Token to SD card.

        // send data to write to SD card.
        for(uint16_t i = 0; i < DATA_BLOCK_LEN; i++)
            sd_SendDataByte(dataBuffer[i]);

        // Send 16-bit CRC. CRC value does not matter if CRC is off (default).
        sd_SendDataByte(0xFF);
        sd_SendDataByte(0xFF);
        

        uint8_t dtMask = 0x1F; //Data Token Mask
        attempt = 0;

        do{ // loop until data response token received.
            
            DataResponseToken = sd_Response(); // get data token
            
            if(attempt++ > 0xFF)
            {
                CS_DEASSERT;
                print_str("\n\r>> ERROR:   Timeout while waiting for Data Response Token in sd_WriteSingleDataBlock().  Returning with INVALID_DATA_TOKEN\n\r");
                return (R1 | INVALID_DATA_RESPONSE);
            }  
             
        }while( ((dtMask&DataResponseToken)!=0x05) &&  // confirm valid data response token received.
                ((dtMask&DataResponseToken)!=0x0B) && 
                ((dtMask&DataResponseToken)!=0x0D) ); 
        
        // Return value corresponding to the data response token received.
        if((DataResponseToken&0x05)==5) // Data Accepted
        {
            int j = 0;
            
            // wait for SD card to not be busy, i.e. to finish writing data to the sector.
            while(sd_Response() == 0) // DO (data out) line held low while card is busy writing data to block.
            {
                if(j > 512) { print_str("\n\r>> timeout waiting for card to not be busy.\n\r"); break; } 
                j++;
            };
            CS_DEASSERT
            return DATA_ACCEPTED;
        }

        else if((DataResponseToken&0x0B)==0x0B) // CRC Error
        {
            CS_DEASSERT;
            print_str("\n\r>> Data rejected Due to CRC Error\n\rData Response Token = ");
            print_hex(DataResponseToken);
            print_str("n\r");
            return CRC_ERROR;
        }

        else if((DataResponseToken&0x0D)==0x0D) // Write Error
        {
            CS_DEASSERT;
            print_str("\n\r>> Data rejected due to a Write Error\n\rData Response Token = ");
            print_hex(DataResponseToken);
            print_str("n\r");
            return WRITE_ERROR;
        }
    }
    return INVALID_DATA_RESPONSE; // successful write returns 0
}
//END sd_WriteSingleDataBlock()



/********************************************************************************
 * Function:    sd_printWriteError(uint16_t err)
 * Description: prints the response returned by sd_WriteSingleDataBlock() in a 
 *              readable form.  Includes the R1 response as well as the value of 
 *              Data Response Token if it is valid.
 * Argument(s): 16-bit response from sd_WriteSingleDataBlock()
 * Returns:     VOID
 * Notes:             
 * ******************************************************************************/
void sd_PrintWriteError(uint16_t err)
{
    //print R1 portion of initiailzation response
    if(SD_MSG > 1) print_str("\n\r>> INFO:    R1 Response returned by sd_WriteSingelDataBlock():");
    sd_printR1((uint8_t)(0x00FF&err));

    if(SD_MSG > 1) print_str("\n\r>> INFO:    Data Response Errors:");
    
    switch(err&0x0700)
    {
        case(INVALID_DATA_RESPONSE):
            print_str("\n\r\t    INVALID DATA RESPONSE");
            break;
        case(WRITE_ERROR):
            print_str("\n\r\t    WRITE_ERROR");
            break;
        case(CRC_ERROR):
            print_str("\n\r\t    CRC_ERROR");
            break;
        case(DATA_ACCEPTED):
            print_str("\n\r\t    DATA ACCEPTED"); // Successful data write
            break;
        default:
            print_str("\n\r\t    INCORRECT RESPONSE RETURNED");
    }
}
//END sd_printWriteError()



/*****************************************************************************
 * Function:    sd_EraseSectors(uint32_t start_address, uint32_t end_address)
 * Description: Erases all sectors between and including start_address and 
 *              end_address.
 * Argument(s): 1) uint32_t start_address: address of first sector to erase.
 *              2) uint32_t end_address:   address of last sector to erase.
 * Returns:     error code. see sd_misc.h
 * Notes:             
 * ***************************************************************************/
uint16_t sd_EraseSectors(uint32_t start_address, uint32_t end_address)
{
    uint8_t R1 = 0;
    
    // set start address for erase block
    CS_ASSERT;
    sd_SendCommand(ERASE_WR_BLK_START_ADDR,start_address);
    R1 = sd_getR1();
    CS_DEASSERT;

    //If R1 is non-zero or times out, then return with R1 response.
    if(R1 > 0)
    {
        print_str("\n\r>> ERROR:   ERASE_WR_BLOCK_START_ADDR (CMD32) in sd_EraseSectors() returned error in R1 response.");
        sd_printR1(R1);
        return (R1 | ERROR_ERASE_START_ADDR);
    }
    
    // set end address for erase block
    CS_ASSERT;
    sd_SendCommand(ERASE_WR_BLK_END_ADDR,end_address);
    R1 = sd_getR1();
    CS_DEASSERT;
    //If R1 is non-zero or times out, then return with R1 response.
    if(R1 > 0)
    {
        print_str("\n\r>> ERROR:   ERASE_WR_BLOCK_END_ADDR (CMD33) in sd_EraseSectors() returned error in R1 response.");
        sd_printR1(R1);
        return (R1 | ERROR_ERASE_END_ADDR);
    }

    // erase all blocks in range of start address to end address
    CS_ASSERT;
    sd_SendCommand(ERASE,0);
    R1 = sd_getR1();
    
    //If R1 is non-zero or times out, then return with R1 response.
    if(R1 > 0)
    {
        CS_DEASSERT;
        print_str("\n\r>> ERROR:   ERASE (CMD38) in sd_EraseSectors() returned error in R1 response.");
        sd_printR1(R1);
        return (R1 | ERROR_ERASE);
    }

    uint16_t attempt = 0; 

    // DO (Data Out) line will be held low by card while it completes erase of data sectors.
    // once it is complete the DO line will be any non-zero value to signal it is out of the busy state.
    while(sd_Response() == 0)
    {
        if(attempt++ > 0xFFFE) 
        {
            print_str("\n\r>> ERROR:    Timeout while waiting for card to exit the busy state during sector erase in sd_EraseSectors(). Returning with error(s).\n\r");
            return (R1 | ERROR_BUSY);
        }
    }
    CS_DEASSERT;
    return ERASE_SUCCESSFUL;
}
// END sd_EraseSectors()



/********************************************************************************
 * Function:    sd_PrintEraseSectorError(uint16_t err)
 * Description: prints the response returned by sd_EraseSector() in readable form.
 *              Includes the R1 response as well as other errors specific to the 
 *              Erase function.
 * Argument(s): uint16_t err: 16-bit response from sd_WriteSingleDataBlock()
 * Returns:     VOID
 * Notes:       
 * ******************************************************************************/
void sd_PrintEraseSectorError(uint16_t err)
{
    //print R1 portion of initiailzation response
    if(SD_MSG > 1) print_str("\n\r>> INFO:    R1 Response returned by sd_EraseSector():");
    sd_printR1((uint8_t)(0x00FF&err));

    if(SD_MSG > 1) print_str("\n\r>> INFO:    sd_EraseSector():");
    
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
//END sd_printWriteError()