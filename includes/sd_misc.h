/******************************************************************************
 * Author: Joshua Fain
 * Date:   7/5/2020
 * 
 * File: SD_MISC.H
 * 
 * Required by: SD_MISC.C
 *
 * Target: ATmega 1280
 * 
 * Description: 
 * Declares functions defined in SD_MISC.C and defines DataSector struct.
 * ***************************************************************************/

#ifndef SD_MISC_H
#define SD_MISC_H

// Struct to store data from data block returned in response to CMD17 along with the R1 response and CRC.
typedef struct  DataSector { //A block is limited to 512 bytes in this implementation
    uint8_t R1;
    uint8_t ERROR;  // no specific errors defined yet.
    uint8_t data[DATA_BLOCK_LEN];
    uint8_t CRC[2];
} DataSector;


// Calculate and return the memory capacity of the SD Card in Bytes.
uint32_t sd_GetMemoryCapacity();



// Reads in a single data secotr from an SD card and returns the data in a 
// DataSector struct.
DataSector sd_ReadSingleDataBlock(uint32_t address);



// Print the data in data sector array passed as the argument.
void sd_PrintSector(uint8_t *sector);  //only 512 byte sector supported.



// Prints multiple data blocks to screen by calling the CMD18.
void sd_PrintMultipleDataBlocks(uint32_t start_address, uint32_t numOfBlocks);



// Prints sectors numbers for those sectors between the begin and end
// sectors that have non-zero data.
void sd_SearchNonZeroSectors(uint32_t begin_sector, uint32_t end_sector);



// Writes data in the dataBuffer to the sector at the address arguement. 
uint16_t sd_WriteSingleDataBlock(uint32_t address, uint8_t *dataBuffer);


/********************************************************************************
 * Data Response Codes: Values returned by the function sd_WriteSingleDataBlock()      
 * Notes:               All, except INVALID_DATA_RESPONSE, correspond to a valid 
 *                      value of the 3-bit data response token returned after an 
 *                      attempt to write data to a single sector/block.
 *                      These are 16-bits here to accommodate the R1 response. 
*********************************************************************************/
#define DATA_ACCEPTED          0x0200
#define CRC_ERROR              0x0500
#define WRITE_ERROR            0x0600
#define INVALID_DATA_RESPONSE  0x0F00 

void sd_printWriteError(uint16_t err);

#endif // SD_MISC_H