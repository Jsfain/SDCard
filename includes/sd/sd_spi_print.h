/*
 * File       : SD_SPI_PRINT.H
 * Version    : 1.0 
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020 - 2024
 * 
 * Interface for printing the various responses that can be returned by SD card 
 * commands - e.g. init, error, and data responses.
 */

/* 
 * ----------------------------------------------------------------------------
 *                                                   ASCII PRINTABLE CHAR RANGE
 *
 * Description : Used to define the printable ascii character range.
 * ----------------------------------------------------------------------------
 */
#define ASCII_PRINT_CHAR_FIRST         32
#define ASCII_PRINT_CHAR_LAST          127

/*
 * ----------------------------------------------------------------------------
 *                                                      PRINT R1 RESPONSE FLAGS
 * 
 * Description : Prints the R1 response flags returned by sd_GetR1().
 * 
 * Arguments   : r1   - The R1 response byte returned by sd_GetR1().
 * ----------------------------------------------------------------------------
 */
void sd_PrintR1(uint8_t r1);

/*
 * ----------------------------------------------------------------------------
 *                                          PRINT INITIALIZATION RESPONSE FLAGS
 * 
 * Description : Prints any Initialization Error Flags returned by 
 *               sd_InitModeSPI during initialization of the SD card.
 * 
 * Arguments   : initResp   - The Initialization Response returned by the
 *                            initialization routine.
 * 
 * Notes       : This function only interprets bits 8 to 16 of the response
 *               returned by sd_InitModeSPI, however, the entire response can
 *               be passed. Bits 0 to 7 represent the R1 response, and will be 
 *               ignored here. To read the R1 portion, pass it to sd_PrintR1.
 * ----------------------------------------------------------------------------
 */
void sd_PrintInitErrorResponse(uint32_t initResp);

/*
 * COMMENT on READ/WRITE/ERASE ERROR Responses
 * The following read, write, and erase block functions will return error
 * responses that can printed via corresponding print error functions. For 
 * example, the returned response of sd_ReadSingleBlock can be printed by 
 * passing it to sd_PrintReadError. These print functions will read the upper
 * byte of the returned error response. If the R1_ERROR flag is set in the 
 * upper byte in any response, then the lower byte (i.e. the R1 Response 
 * portion of the error response) contains at least one flag that has been set
 * which can then be read by passing it to sd_printR1 defined in SD_SPI_BASE.
 */ 


/*
 * ----------------------------------------------------------------------------
 *                                                             PRINT READ ERROR
 * 
 * Description : Print Read Error Flag returned by a SD card read function.  
 * 
 * Arguments   : err   - Read Error Response.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */
void sd_PrintReadError(uint8_t err);

/*
 * ----------------------------------------------------------------------------
 *                                                            PRINT WRITE ERROR
 * 
 * Description : Print Write Error Flag returned by a SD card read function.  
 * 
 * Arguments   : err   - Write Error Response.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */
void sd_PrintWriteError(uint8_t err);

/*
 * ----------------------------------------------------------------------------
 *                                                            PRINT ERASE ERROR
 * 
 * Description : Print Erase Error Flag returned by a SD card read function.  
 * 
 * Arguments   : err   - Erase Error Response.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */
void sd_PrintEraseError(uint16_t err);


/*
 * ----------------------------------------------------------------------------
 *                                                           PRINT SINGLE BLOCK
 * 
 * Description : Print contents of an array loaded with data from a single
 *               block on the SD card. The array's contents will be printed to 
 *               the screen in rows of 16 data bytes. Each row begins with the
 *               block offset address of the first byte in each row, a copy of 
 *               the data in HEX format and a copy of the data in ASCII.
 * 
 * Arguments   : blckArr   - pointer to array containing contents of the data 
 *                           block to be printed to screen. 
 * 
 * Note        : Array must be of length BLOCK_LEN.
 * ----------------------------------------------------------------------------
 */
void sd_PrintSingleBlock(const uint8_t blckArr[]);