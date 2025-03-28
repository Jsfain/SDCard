/*
 * File       : SD_SPI_RWE.H
 * Version    : 1.0 
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020 - 2025
 * 
 * Interface for SD Card single-block (R)ead, (W)rite and multi-block (E)rase.
 */

#ifndef SD_SPI_RWE_H
#define SD_SPI_RWE_H

/*
 ******************************************************************************
 *                                  MACROS   
 ******************************************************************************
 */

/* 
 * ----------------------------------------------------------------------------
 *                                                                  DUMMY TOKEN
 *
 * Description : Token sent to the SD card when the value of the data sent
 *               doesn't matter.
 * ----------------------------------------------------------------------------
 */
#define DMY_TKN                        0xFF

/* 
 * ----------------------------------------------------------------------------
 *                                                            START BLOCK TOKEN
 *
 * Description : Token sent by the SD card to signal it is ready to send or
 *               receive data for a block read/write.
 * ----------------------------------------------------------------------------
 */
#define START_BLOCK_TKN                0xFE

/* 
 * ----------------------------------------------------------------------------
 *                                                         DATA RESPONSE TOKENS
 *
 * Description : Used for data responses during a write operation. These tokens
 *               are sent by the SD card to indicate whether the data was
 *               accepted for write or an error occurred. The returned byte 
 *               containing the response token is of the form - XXX0TTT1, where 
 *               X's are 'don't cares', and T's are token bits. The mask
 *               filters the response to extract the token.
 * ----------------------------------------------------------------------------
 */
#define DATA_ACCEPTED_TKN              0x05
#define CRC_ERROR_TKN                  0x0B
#define WRITE_ERROR_TKN                0x0D
#define DATA_RESPONSE_TKN_MASK         0x1F

/* 
 * ----------------------------------------------------------------------------
 *                                                                R1 ERROR FLAG
 *
 * Description : Flag used to indicate a functions's returned error is the R1 
 *               response and not one of the other errors defined here.
 * ----------------------------------------------------------------------------
 */
#define R1_ERROR                       0x8000

/* 
 * ----------------------------------------------------------------------------
 *                                                       READ BLOCK ERROR FLAGS
 *
 * Description : non-R1 response flags returned by READ block functions.
 * ----------------------------------------------------------------------------
 */
#define READ_SUCCESS                   0x01
#define START_TOKEN_TIMEOUT            0x02


/* 
 * ----------------------------------------------------------------------------
 *                                                      WRITE BLOCK ERROR FLAGS
 *
 * Description : non-R1 response flags returned by WRITE block functions.
 * ----------------------------------------------------------------------------
 */
#define WRITE_SUCCESS                  0x01
#define CRC_ERROR_TKN_RECEIVED         0x02
#define WRITE_ERROR_TKN_RECEIVED       0x04
#define INVALID_DATA_RESPONSE          0x08
#define DATA_RESPONSE_TIMEOUT          0x10
#define CARD_BUSY_TIMEOUT              0x20

/* 
 * ----------------------------------------------------------------------------
 *                                                      ERASE BLOCK ERROR FLAGS
 *
 * Description : non-R1 response flags returned by ERASE functions.
 * 
 * Notes       : The lower byte is reserved for the R1 Response Flags (see
 *               SD_SPI_BASE.H).
 * ----------------------------------------------------------------------------
 */
#define ERASE_SUCCESS                  0x0100
#define SET_ERASE_START_ADDR_ERROR     0x0200
#define SET_ERASE_END_ADDR_ERROR       0x0400
#define ERASE_ERROR                    0x0800
#define ERASE_BUSY_TIMEOUT             0x1000

/*
 ******************************************************************************
 *                               FUNCTIONS   
 ******************************************************************************
 */

/*
 * ----------------------------------------------------------------------------
 *                                                            READ SINGLE BLOCK
 * 
 * Description : Reads a single data block from the SD card into an array.     
 * 
 * Arguments   : blckAddr   - address of the data block on the SD card that    
 *                            will be read into the array.
 *               blckArr    - pointer to the array to be loaded with the       
 *                            contents of the data block at blckAddr. The array
 *                            must be length BLOCK_LEN.
 * 
 * Returns     : If an R1 error occurs when sending the READ command the 
 *               returned response is the R1 error and the R1_ERROR 
 *               flag is set to indicate this. If no R1 error occurs, the 
 *               function returns one of the READ BLOCK ERROR flags.   
 * ----------------------------------------------------------------------------
 */
uint16_t sd_ReadSingleBlock(uint32_t blckAddr, uint8_t blckArr[]);

/*
 * ----------------------------------------------------------------------------
 *                                                           WRITE SINGLE BLOCK
 * 
 * Description : Writes the values in an array to a single SD card data block.
 * 
 * Arguments   : blckAddr   - address of the data block on the SD card that 
 *                            will be written to.
 *               dataArr    - pointer to an array that holds the data contents
 *                            that will be written to the block at blckAddr on
 *                            the SD card. Must be of length BLOCK_LEN.
 * 
 * Returns     : If an R1 error occurs when sending the WRITE command, the 
 *               returned response is the R1 error and the R1_ERROR 
 *               flag is set to indicate this. If no R1 error occurs, the 
 *               function returns one of the WRITE BLOCK ERROR flags.   
 * ----------------------------------------------------------------------------
 */
uint16_t sd_WriteSingleBlock(uint32_t blckAddr, const uint8_t dataArr[]);

/*
 * ----------------------------------------------------------------------------
 *                                                                 ERASE BLOCKS
 * 
 * Description : Erases the blocks between (and including) startBlckAddr and
 *               endBlckAddr.
 * 
 * Arguments   : startBlckAddr   - address of the first block to be erased.
 *               endBlckAddr     - address of the last block to be erased.
 * 
 * Returns     : Erase Block Error is returned in the upper byte and if an R1
 *               response other OUT_OF_IDLE is returned by the SD card when
 *               issuing one of the in this function, then the R1_ERROR flag is
 *               also set and the lower byte will contain this R1 response.
 * ----------------------------------------------------------------------------
 */
uint16_t sd_EraseBlocks(uint32_t startBlckAddr, uint32_t endBlckAddr);


#endif // SD_SPI_RWE_H
