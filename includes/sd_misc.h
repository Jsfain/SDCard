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
 * Declares functions defined in SD_MISC.C and defines DataBlock struct.
 * ***************************************************************************/

#ifndef SD_MISC_H
#define SD_MISC_H

// Struct to store data from data block returned in response to CMD17 along with the R1 response and CRC.
typedef struct  DataBlock { //A block is limited to 512 bytes in this implementation
    uint8_t R1;
    uint8_t ERROR;  // no specific errors defined yet.
    uint8_t data[DATA_BLOCK_LEN];
    uint8_t CRC[2];
} DataBlock;



// Calculate and return the memory capacity of the SD Card in Bytes.
uint32_t sd_GetMemoryCapacity();



// Reads in a single data block from an SD card and returns it in a DataBlock struct.
DataBlock sd_ReadSingleDataBlock(uint32_t address);



// Print the data in data block array passed as the argument.
void sd_PrintDataBlock(uint8_t *block);  //only 512 byte block supported.



// Prints multiple data blocks to screen by calling the CMD18.
void sd_PrintMultipleDataBlocks(uint32_t start_address, uint32_t numOfBlocks);



// Prints blocks numbers for those blocks between the begin and end
// blocks that have non-zero data.
void sd_SearchNonZeroBlocks(uint32_t begin_block, uint32_t end_block);



/********************************************************************************
 * Data Response Codes: Values returned by the function sd_WriteSingleDataBlock()      
 * Notes:               All, except INVALID_DATA_RESPONSE, correspond to a valid 
 *                      value of the 3-bit data response token returned after an 
 *                      attempt to write data to a single block.
 *                      These are 16-bits here to accommodate the R1 response. 
*********************************************************************************/
#define DATA_ACCEPTED          0x0200
#define CRC_ERROR              0x0500
#define WRITE_ERROR            0x0600
#define INVALID_DATA_RESPONSE  0x0F00 


// Writes data in the dataBuffer to the block at the address arguement. 
uint16_t sd_WriteSingleDataBlock(uint32_t address, uint8_t *dataBuffer);



// Writes data in the dataBuffer to the multiple blocks beginning at address. 
uint16_t sd_WriteMultipleDataBlocks(uint32_t address, uint8_t nob, uint8_t *dataBuffer);



// Prints the error code returned by sd_WriteSingleDataBlock() in readable format.
void sd_PrintWriteError(uint16_t err);


// returns the number of well written blocks after a multi-block write operation is performed.
uint32_t sd_NumberOfWellWrittenBlocks();



/********************************************************************************
 * Error Flags:         Values returned by the function sd_Eraseblocks()      
*********************************************************************************/
#define ERASE_SUCCESSFUL       0x0000
#define ERROR_ERASE_START_ADDR 0x0100
#define ERROR_ERASE_END_ADDR   0x0200
#define ERROR_ERASE            0x0400
#define ERROR_BUSY             0x0800


// erases blocks from start_address to end_address (inclusive)
uint16_t sd_EraseBlocks(uint32_t start_address, uint32_t end_address);


// print error code returned by sd_Eraseblocks()
void sd_PrintEraseBlockError(uint16_t err);

#endif // SD_MISC_H