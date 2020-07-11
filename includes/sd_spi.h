/******************************************************************************
 * Author: Joshua Fain
 * Date:   7/5/2020
 * 
 * File: SD_SPI.H
 * 
 * Required by: SD_SPI.C
 *
 * Target: ATmega 1280
 * 
 * Description: 
 * Declares SD Card "base-level" functions, defines SD Card commands as well as
 * various error, and other, flags required by SD_SPI.C.  See comments
 * throughout file for specifics.
 * ***************************************************************************/


#ifndef SD_SPI_H
#define SD_SPI_H

#include "../includes/spi.h"

/******************************************************************************
 * Flag:        SD_MSG 
 * Description: Useful for debugging, this flag can be used to print differnt
 *              messages to the screen in functions defined or derived from here
 * Settings:    0  = NO messages
 *              1  = ERROR messages
 *              2  = INFO messages
 *              3  = DEBUG messages
 *             >3  = VERBOSE messages
 * Notes:       Call sd_printInitResponse() to read initialization response.
******************************************************************************/
#define SD_MSG 2

/******************************************************************************
 * Flag:        HCS 
 * Description: Indicates card capacity supported by host. Currently this
 *              should always be set to 0.
 * Settings:    0 = host supports SDSC. 
                1 = host supports SDHC or SDXC.
******************************************************************************/
#define HCS 0 //
              


/******************************************************************************
 * Flag:        DATA_BLOCK_LEN
 * Description: Defines the data block length.
 * Settings:    512
******************************************************************************/
#define DATA_BLOCK_LEN 512


/******************************************************************************
 * Functions:   ASSERT / DEASSERT       
 * Description: ASSERT will assert (bring low) the SS pin of the SPI port 
 *              (CS pin of SD Card) to signal the SD Card to receive a command/
 *              return a response. DEASSERT when command/response is complete.
******************************************************************************/
#define CS_ASSERT    SPI_PORT &= ~(1<<SS);
#define CS_DEASSERT  SPI_PORT |= (1<<SS);


/******************************************************************************
 * Description: The SD Card commands available in SPI Mode.     
******************************************************************************/
#define GO_IDLE_STATE               0       //CMD0
#define SEND_OP_COND                1       //CMD1
#define SWITCH_FUNC                 6       //CMD6
#define SEND_IF_COND                8       //CMD8
#define SEND_CSD                    9       //CMD9
#define SEND_CID                    10      //CMD10
#define STOP_TRANSMISSION           12      //CMD12
#define SEND_STATUS                 13      //CMD13
#define SET_BLOCKLEN                16      //CMD16
#define READ_SINGLE_BLOCK           17      //CMD17
#define READ_MULTIPLE_BLOCK         18      //CMD18
#define WRITE_BLOCK                 24      //CMD24
#define WRITE_MULTIPLE_BLOCK        25      //CMD25
#define PROGRAM_CSD                 27      //CMD27
#define SET_WRITE_PROT              28      //CMD28
#define CLR_WRITE_PROT              29      //CMD29
#define SEND_WRITE_PROT             30      //CMD30
#define ERASE_WR_BLK_START_ADDR     32      //CMD32
#define ERASE_WR_BLK_END_ADDR       33      //CMD33
#define ERASE                       38      //CMD38
#define LOCK_UNLOCK                 42      //CMD42
#define APP_CMD                     55      //CMD55
#define GEN_CMD                     56      //CMD56
#define READ_OCR                    58      //CMD58
#define CRC_ON_OFF                  59      //CMD59
// Application Specific Commands. To activate, first call APP_CMD.
#define SD_STATUS                   13      //ACMD13
#define SEND_NUM_WR_BLOCKS          22      //ACMD22
#define SET_WR_BLK_ERASE_COUNT      23      //ACMD23
#define SD_SEND_OP_COND             41      //ACMD41
#define SET_CLR_CARD_DETECT         42      //ACMD42
#define SEND_SCR                    51      //ACMD51



/******************************************************************************
 * Flags:       R1 Response Flags       
 * Description: Flags used to interpret the R1 response.  
 * Notes:       R1_TIMEOUT is not part of the specification but occupies the
 *              reserved bit [8]. Specified as 5 bytes to be used in
 *              conjunction with other error flags.
******************************************************************************/
#define OUT_OF_IDLE             0x00000   // No errors and out of idle state
#define IN_IDLE_STATE           0x00001
#define ERASE_RESET             0x00002
#define ILLEGAL_COMMAND         0x00004
#define COM_CRC_ERROR           0x00008
#define ERASE_SEQUENCE_ERROR    0x00010
#define ADDRESS_ERROR           0x00020
#define PARAMETER_ERROR         0x00040
#define R1_TIMEOUT              0x00080   //Not part of SD Specification. Should be set in code



/******************************************************************************
 * Flags:       Initialization response flags returned by sd_SPI_Mode_Init().      
 * Description: Returned by sd_SPI_Mode_Init(). 
******************************************************************************/
#define FAILED_GO_IDLE_STATE    0x00100   //CMD0
#define FAILED_SEND_IF_COND     0x00200   //CMD8
#define UNSUPPORTED_CARD_TYPE   0x00400   //Card version and/or capacity not supported
#define FAILED_CRC_ON_OFF       0x00800   //CMD59
#define FAILED_APP_CMD          0x01000   //CMD55
#define FAILED_SD_SEND_OP_COND  0x02000   //ACMD41
#define OUT_OF_IDLE_TIMEOUT     0x04000
#define FAILED_READ_OCR         0x08000   //CMD58
#define POWER_UP_NOT_COMPLETE   0x10000
#define FAILED_SET_BLOCKLEN     0x20000
#define FAILED_SEND_CSD         0x40000




/******************************************************************************
 * Functions: See the full description of these function in SD_SPI.C
******************************************************************************/


//initializes SD card in SPI mode
uint32_t sd_SPI_Mode_Init();

//sends one byte of data to the SD card via SPI.
void sd_SendByte(uint8_t data);

//sends SD command/argument to SD Card
void sd_SendCommand(uint8_t cmd, uint32_t arg);

//gets 8 bit response returned from SD card
uint8_t sd_ReturnByte();

//calculates the CRC7 bits for command/argument combination
uint8_t sd_CRC7(uint64_t ca);

//gets R1 response
uint8_t sd_getR1();

//prints R1 response
void sd_printR1(uint8_t R1);

//prints the response returned by sd_SPI_Mode_Init()
void sd_printInitResponse(uint32_t err);


#endif //SD_SPI_H
