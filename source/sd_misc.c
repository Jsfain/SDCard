#include <stdint.h>
#include <avr/io.h>
#include "../includes/usart.h"
#include "../includes/prints.h"
#include "../includes/spi.h"
#include "../includes/sd_base.h"
#include "../includes/sd_misc.h"




//calculates and returns the memory capacity of the SD card from CSD parameters.
//Returns 1 if there is an error getting or calculating the value.
uint32_t sd_getMemoryCapacity()
{
    //Memory Capacity is calculated from the C_SIZE, READ_BL_LEN, and C_SIZE_MULT parameters found in the CSD register.
    //This function reads in the CSD register and parses the returned values, performing checks on the returned values of the CSD where possible.
    //It then calculates the memory capacity based on the equation supplied in the Part1_Physical_Layer Specification.

    if(SD_MSG > 2) print_str("\n\r>> INFO: In sd_getMemoryCapacity()\n\r");

    uint8_t READ_BL_LEN = 0;
    uint16_t C_SIZE = 0;
    uint8_t C_SIZE_MULT = 0;

    uint8_t resp;
    uint8_t R1 = 0;
    uint8_t attempt = 0;

    CS_ASSERT;

    for(int i=0;i<=20;i++) SPI_MasterTransmit(0xFF); //Calling SEND_CSD too quickly after calling from sd_SPI_Mode_Init() 
                                                     //required several more dummy data to be sent to ensure the card was.

    sd_SendCommand(SEND_CSD,0); //CMD9 Request SD card return CSD.

    //If R1 is non-zero or times out, then return without getting rest of CSD.
    while((R1 = sd_Response())==0xFF) 
    {  
        if(attempt++ >= 0xFF)
        {
            CS_DEASSERT
            print_str("\n\r>> ERROR: Timeout on R1 response for SEND_CSD (CMD9) in sd_getMemoryCapacity(). Returning with value 1.\n\r");
            return 1;
        }
    }
    if(SD_MSG > 0) {print_str("\n\r>> DEBUG: printing R1 response to SEND_CSD (CMD9) in sd_getMemoryCapacity().\n\r"); sd_printR1(R1);}

    if(R1 > 1)
    {
        CS_DEASSERT
        print_str(">> Error returned in R1 response of SEND_CSD (CMD9) in sd_getMemoryCapacity(). Returning with value 1.\n\r");
        return 1;
    }

    attempt = 0;
    do{ // CSD_STRUCTURE
        if((sd_Response()>>6) <= 2) break;
        if(attempt++ >= 0xFF){ CS_DEASSERT; return 1;} // Timeout returning CSD_STRUCTURE 
    }while(1);
    
    attempt = 0;
    do{ // TAAC
        if(!(sd_Response()>>7)) break;
        if(attempt++ >= 0xFF) { CS_DEASSERT; return 1;} //Timeout returning TAAC
    }while(1);

    sd_Response(); //NSAC of CSD. Any value could be valid.

    attempt = 0;
    do{ //TRAN_SPEED. Must either be 0x32 or 0x5A.
        resp = sd_Response();  
        if((resp == 0x32) || (resp == 0x5A)) break;
        if(attempt++ >= 0xFF) { CS_DEASSERT; return 1;}
    }while(1);

    //Next bytes include 12 bit CCC and 4 bit READ_BL_LENGTH.  READ_BL_LEN is needed to calculate memory capacity.
    //CCC is of the form 01_110110101. READ_BL_LEN is 4 bits and must by 9, 10, or 11.
    uint8_t flag = 1;

    while(flag == 1)
    {
        if((resp = sd_Response())&0x7B) //CCC 8 most significant bits must be of the form 01_11011;
        {
            if(((resp = sd_Response())>>4) == 0x05) //CCC least sig. 4 bits and 4 bit READ_BL_LEN.
            {
                READ_BL_LEN = resp&0b00001111;
                if ((READ_BL_LEN < 9) || (READ_BL_LEN > 11))
                {
                    CS_DEASSERT;
                    print_str("\n\r>> FAILED to get valid READ_BL_LEN value in sd_getMemoryCapacity().");
                    print_str("\n\r>> READ_BL_LEN should be 0x09, 0x0A, or 0x0B, but the value returned was 0x"); print_hex(READ_BL_LEN);
                    print_str("\n\r>> Returning with value 1.");
                    return 1; 
                }
                flag = 0;
            }
        }
        if(attempt++ >= 0xFF) { CS_DEASSERT; return 1;}
    }


    //This section gets the remaining bits of the CSD
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
                                        //C_SIZE - 2 Most Significant Bits[1:0] = X;
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

    // Calculate memory capacity of SD Card.
    uint32_t BLOCK_LEN = 1;
    for (uint8_t i = 0; i< READ_BL_LEN; i++) BLOCK_LEN = BLOCK_LEN*2;
    uint32_t MULT = 1;
    for (uint8_t i = 0; i< C_SIZE_MULT+2; i++) MULT = MULT*2;
    uint32_t BLOCKNR = (C_SIZE+1)*MULT;
    uint32_t memoryCapacity = BLOCKNR*BLOCK_LEN;
    
    return memoryCapacity;

}
//END sd_MemoryCapacity()



// Reads data sector
DataSector sd_ReadSingleDataSector(uint32_t address)
{
    //print_str("\n\rIn READ Single Data Block\n\r");
    DataSector ds;

    int attempt = 0;

    CS_ASSERT;
    for(int i=0;i<2;i++) SPI_MasterTransmit(0xFF);	//Wait 16 clock more clock cycles.  

    sd_SendCommand(READ_SINGLE_BLOCK,address);  //Send CMD17;
    while((ds.R1 = sd_Response())==0xFF) 
    {  
        if(attempt++ >= 0xFF)
        {
            print_str("took too long\n\r");
            sd_printR1(ds.R1);
            return ds;
        }
    }
    //sd_printR1(ds.R1);

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
                print_str("data read time out");
                ds.ERROR = 1;
                return ds;
            }
        }

        if((attempt < 0xFF) && (r== 0xFE))
        {            
            for(uint16_t i = 0; i < DATA_BLOCK_LEN; i++)
                ds.byte[i] = sd_Response();

            for(int i = 0; i < 2; i++)
                ds.CRC[i] = sd_Response();
                
            sd_Response();
        }
    }
    CS_DEASSERT;
    return ds;
}
//END sd_ReadSingleDataSector()



void print_sector(uint8_t *sector)
{
    print_str("\n\n\rPRINTING DATA FOR 512-byte SECTOR\n\r");

    print_str("\n\n\rSECTOR OFFSET\t\t\t\t   HEX\t\t\t\t\t     ASCII\n\r");
    uint16_t sector_offset = 0;
    int space = 0;
    for(uint16_t row=0; row<DATA_BLOCK_LEN/16; row++)
    {
        print_str("\n\r   ");
        //if(sector_offset<0x10)        { print_str("0x0"); print_hex(sector_offset); }
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
