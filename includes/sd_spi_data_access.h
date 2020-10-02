/******************************************************************************
 * SD_SPI_DATA_ACCESS.H
 *  
 * TARGET
 * ATmega 1280
 *
 * DESCRIPTION
 * Uses SD_SPI_BASE.C(H) to implement specialized functions against an SD card
 * hosted on an AVR microcontroller and operating in SPI mode such as reading,
 * writing, erasing, and printing raw data blocks. The physical interaction
 * with the SD card is handled by functions defined in SD_SPI_BASE
 *  
 * 
 * Author: Joshua Fain
 * Date:   9/17/2020
 * ***************************************************************************/



#ifndef SD_SPI_DATA_ACCESS_H
#define SD_SPI_DATA_ACCESS_H



/******************************************************************************
 *                          MACROS / DEFINITIONS / FLAGS
 *****************************************************************************/


 
// when used in a returned response, this flag indicates the returned
// value has an error in the R1 response portion of the response.
#define R1_ERROR               0x8000



// Read Block Error Flags
#define START_TOKEN_TIMEOUT    0x0200
#define READ_SUCCESS           0x0400
//#define R1_ERROR             0x8000 //defined above



// Write Block Error Flags
#define DATA_ACCEPTED_TOKEN_RECEIVED    0x0100
#define CRC_ERROR_TOKEN_RECEIVED        0x0200
#define WRITE_ERROR_TOKEN_RECEIVED      0x0400
#define INVALID_DATA_RESPONSE           0x0800
#define DATA_RESPONSE_TIMEOUT           0x1000
#define CARD_BUSY_TIMEOUT               0x2000
//#define R1_ERROR                      0x8000 //defined above




// Error Error Flags      
#define ERASE_SUCCESSFUL           0x0000
#define SET_ERASE_START_ADDR_ERROR 0x0100
#define SET_ERASE_END_ADDR_ERROR   0x0200
#define ERASE_ERROR                0x0400
#define ERASE_BUSY_TIMEOUT         0x0800
//#define R1_ERROR                 0x8000 //defined above


/******************************************************************************
 *                          FUNCTION DECLARATIONS
 *****************************************************************************/

// Read single block at address from SD card into array.
uint16_t SD_ReadSingleBlock(uint32_t blockAddress, uint8_t *block);


// Print columnized (address) OFFSET | HEX | ASCII values in *byte array
void SD_PrintSingleBlock(uint8_t *block);


// Writes data in array pointed at by *data to the block at 'address'
uint16_t SD_WriteSingleBlock(uint32_t blockAddress, uint8_t *data);


// Erases blocks from start_address to end_address (inclusive)
uint16_t SD_EraseBlocks(uint32_t startBlockAddress, uint32_t endBlockAddress);


// Print multiple data blocks to screen.
uint16_t SD_PrintMultipleBlocks(
                uint32_t startBlockAddress, 
                uint32_t numberOfBlocks);


// Writes data in the array pointed at by *data to multiple blocks
// specified by 'numberOfBlocks' and starting at 'address'.
uint16_t SD_WriteMultipleBlocks(
                uint32_t blockAddress, 
                uint32_t numberOfBlocks, 
                uint8_t *dataBuffer);


// Gets the number of well written blocks after multi-block write error
uint16_t SD_GetNumberOfWellWrittenBlocks(uint32_t *wellWrittenBlocks);


// Prints the error code returned by a read function.
void SD_PrintReadError(uint16_t err);


// Prints the error code returned by a write function.
void SD_PrintWriteError(uint16_t err);


// Prints the error code returned by erase blocks function.
void SD_PrintEraseError(uint16_t err);



#endif // SD_SPI_DATA_ACCESS_H
