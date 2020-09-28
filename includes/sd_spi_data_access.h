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



// Struct whose members are the raw data bytes, R1 
// response, and CRC returned by a data block read.
typedef struct {
    uint8_t byte[BLOCK_LEN]; // 512 byte array to hold block data bytes
    uint8_t CRC[2];
} Block;


// Error flags / tokens used explicitly by functions defined here.
// The lowest byte in each flag / token is 0 as this byte will be
// occupied by the R1 response in the returned values.
// Call the specific 'Print Error' function on the upper bytes to 
// to print specific errors here, call 'SD_PrintR1(r1) on the lowest
// byte to print the R1 response.


 
// used to indicate the returned value has 
// an error in the R1 response portion.
#define R1_ERROR               0x8000

// Read Block Error Flags
#define START_TOKEN_TIMEOUT    0x0200
#define READ_SUCCESS           0x0400


// Error responses returned by sd_WriteSingleDataBlock(). All except 
// INVALID_DATA_RESPONSE, correspond to valid value of the 3-bit data response 
// token returned after an attempt to write data to a single block. Low byte will
// hold the R1 response.
#define DATA_ACCEPTED_TOKEN             0x0100
#define CRC_ERROR_TOKEN                 0x0200
#define WRITE_ERROR_TOKEN               0x0400
#define INVALID_DATA_RESPONSE           0x0800
#define DATA_RESPONSE_TIMEOUT           0x1000
#define CARD_BUSY_TIMEOUT               0x2000
//#define R1_ERROR                      0x8000 //defined above


//Well Written Block Error Flags


// Error responses returned by the function sd_Eraseblocks()      
#define ERASE_SUCCESSFUL           0x0000
#define SET_ERASE_START_ADDR_ERROR 0x0100
#define SET_ERASE_END_ADDR_ERROR   0x0200
#define ERROR_ERASE                0x0400
#define ERASE_BUSY_TIMEOUT         0x0800



/******************************************************************************
 *                          FUNCTION DECLARATIONS
 *****************************************************************************/


/******************************************************************************
 * Function:    sd_ReadSingleBlock(uint32_t address)
 * Description: Reads in a single data block from the SD card at 'address'
 *              and stores in a DataBlock struct's data[] member.
 * Argument:    Address of data block to read = 512 * Block or Sector number
 * Returns:     DataBlock object.
******************************************************************************/
uint16_t SD_ReadSingleBlock(uint32_t blockAddress, Block *ds);


/******************************************************************************
 * Function:    sd_PrintBlock(uint8_t *block)
 * Description: Prints the contents of the SD data block array pointed at by 
 *              *block. The block's contents are printed in rows of 32 bytes 
 *              each. The byte contents are printed in hexadecimal and ASCII
 *              characters (if valid).  The beginning of each row provides the
 *              relative byte offset within the block of the first byte in the
 *              row.
 * Argument     uint8_t pointer to a 512 byte array holding the contents of a
 *              block of data from the SD card.
 * Returns:     VOID
 * Notes:       For the ASCII characters, a ' ' will be printed if value is
 *              < 32, '.' if > 128 and the ASCII character otherwise.
******************************************************************************/
void SD_PrintBlock(uint8_t *byte);  //only 512 byte block supported.



void SD_PrintReadError(uint16_t err);



/******************************************************************************
 * Function:    sd_PrintMultipleBlocks(
 *                      uint32_t start_address, 
 *                      uint32_t numOfBlocks)
 * Description: Prints multiple, consecutive data blocks using by requesting 
 *              sd_SendCommand send the READ_MULTIPLE_BLOCK command, and prints
 *              the returned blocks to the screen by calling 
 *              sd_PrintBlock(). The range of data blocks to be printed 
 *              begin at start_address and end at 
 *              start_address + (numOfBlocks - 1).
 * Argument:    1) uint32_t start address (512 * block number) 
 *              2) uint32_t number of blocks to be read in and printed.
 * Returns:     VOID
******************************************************************************/
uint16_t SD_PrintMultipleBlocks(
                    uint32_t startBlockAddress, 
                    uint32_t numberOfBlocks);



/******************************************************************************
 * Function:    sd_WriteSingleDataBlock(uint32_t address, uint8_t *dataBuffer)               
 * Description: Writes the data in the 512 byte array pointed at by *dataBuffer
 *              to the block at 'address'.
 * Arguments:   1) address - uint32_t address of the data block to written.
 *              2) *dataBuffer - uint8_t pointer to a 512 byte array holdin
 *                      the values to print to the block at 'address'.
 * Returns:     uint16_t error code whose value is the Data Response Code in 
 *              the MSByte and the most recent R1 response in the LSByte.
 * Notes:       1) Byte returned by SD Card for the Data Response Token is of
 *                 the form xxx0SSS1, where:
 *                              xxx = don't care 
 *                              SSS = 010: Data Accepted
 *                              SSS = 101: Data rejected due to a CRC Error.
 *                              SSS = 101: Data rejected due to a Write Error.
 * 
 *              2) If the returned value indicates a write error occurred then 
 *                 the SEND_STATUS command should be sent in order to get the 
 *                 cause of the write error.  
******************************************************************************/
uint16_t SD_WriteSingleDataBlock(uint32_t blockAddress, uint8_t *data);



/******************************************************************************
 * Function:    sd_WriteMultipleDataBlocks(
 *                      uint32_t address, 
 *                      uint8_t  nob,
 *                      uint8_t *dataBuffer)
 * Description: Writes the data in the array pointed at by *dataBuffer to all 
 *              blocks consecutively in the range of [address:address + nob] 
 *              (inclusive). Currently this will write the same data in the 
 *              dataBuffer to every block in the block range.
 * Arguments:   1) address - uint32_t address of first byte in first block
 *                  that will be written to.
 *              2) nob - uint8_t number of blocks to write to.
 *              3) *dataBuffer - uint8_t pointer to an array of data that
 *                  will be written to the SD card.
 * Returns:     uint16_t error code whose value is the Data Response Code in 
 *              the MSByte and the most recent R1 response in the LSByte.
 * Notes:       1) Byte returned by SD Card for the Data Response Token is of
 *                 the same form specified in sd_WriteSingleDataBlock().
 *              2) If returned value indicates a write error occurred then the 
 *                 SEND_STATUS command should be sent in order to get the cause
 *                 of the write error.  
 *              3) Call ACMD22 after this completes to get the number blocks
 *                 that were successfully written to.
******************************************************************************/
uint16_t SD_WriteMultipleDataBlocks(
                uint32_t blockAddress, 
                uint32_t numberOfBlocks, 
                uint8_t *dataBuffer);



void SD_PrintWriteError(uint16_t err);



/******************************************************************************
 * Function:    sd_printWriteError(uint16_t err)
 * Description: Prints the response returned by sd_WriteSingleDataBlock().
 *              Includes the R1 response as well as the value of the Data 
 *              Response Token, if it is valid.
 * Argument:    err - uint16_t error response from sd_WriteSingleDataBlock()
 * Returns:     VOID
 * ***************************************************************************/
void SD_PrintWriteError(uint16_t err);



/******************************************************************************
 * Function:    sd_NumberOfWellWrittenBlocks(void)
 * Description: Returns the number of well written blocks after a multi-block 
 *              write operation is performed. Call this function when 
 *              sd_WriteMultipleDataBlocks() returns a write error to see how
 *              many blocks were successfully written to.
 * Argument(s): VOID
 * Returns:     uint32_t number of well written blocks. 0 if error.
 * ***************************************************************************/
uint16_t SD_NumberOfWellWrittenBlocks(uint32_t *wellWrittenBlocks);



/******************************************************************************
 * Function:    sd_EraseBlocks(uint32_t start_address, uint32_t end_address)
 * Description: Erases all blocks between, and including, the blocks in the  
 *              interval between start_address and end_address.
 * Argument(s): 1) start_address - uint32_t value of the first block to erase.
 *              2) end_address - uint32_t value of the last block to erase.
 * Returns:     uint16_t error code whose value is the Data Response Code in 
 *              the MSByte and the most recent R1 response in the LSByte.       
 * ***************************************************************************/
uint16_t SD_EraseBlocks(uint32_t startBlockAddress, uint32_t endBlockAddress);



/********************************************************************************
 * Function:    sd_PrintEraseError(uint16_t err)
 * Description: Prints the response returned by sd_EraseBlocks() in readable form.
 *              Includes the R1 response as well as other errors specific to the 
 *              Erase function.
 * Argument(s): err - uint16_t error response from sd_WriteSingleDataBlock()
 * Returns:     VOID
 * ******************************************************************************/
void SD_PrintEraseError(uint16_t err);



#endif // SD_SPI_DATA_ACCESS_H
