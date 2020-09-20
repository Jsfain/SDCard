# SD Card Module
Used to execute the SPI mode SD card command set using an AVR host.

# Purpose
To establish a set of functions for access and control of an SD card by an AVR microcontroller. This module is intended to be used as a phyiscal disk layer operating underneath a file system module, however, it can simply be used by itself for raw data access (read/write/erase) and disk control as it is possible to execute any valid SD Card SPI Mode command from this module.

# Details
TARGET: AVR ATmega1280 microcontroller
LANGUAGE: C 
COMPILER: Successfully compiles with avr-gcc

# Overview
* The SD Card Module is separated into of base functions (SD_SPI_BASE) and special functions (SD_SPI_SF), requiring the base functions.

* SD_SPI_BASE contains the base functions that are required by any implementation of this module. Its role is to handle direct phyical interaction with the SD card and includes routines to accomplish the following tasks:
  * Initialization of the SD card into SPI Mode.
  * Sending SPI-specific SD card commands/arguments to the SD card.
  * Receiving the card's returned bytes in response to any command.
  * Error printing.

* SD_SPI_SF contains some special functions, requiring SD_SPI_BASE, and include routines for handling tasks such as:
  * Reading / Writing / Erasing data blocks.
  * Block print functions.
  * Others.

# Detailed Description
SD_SPI_BASE.H

DESCRIPTION
Base-level SD card functions, objects, and definition MACROS used for 
physical interaction with an SD card operating SPI Mode.

FUNCTION LIST
1) uint32_t  sd_SPI_Mode_Init(void)
2) void      sd_SendByte(uint8_t data)
3) uint8_t   sd_ReturnByte(void)
4) void      sd_SendCommand(uint8_t cmd, uint32_t arg)
5) uint8_t   sd_CRC7(uint64_t tca)
6) uint8_t   sd_getR1(void) 
7) void      sd_printR1(uint8_t R1)
8) void      sd_printInitResponse(uint32_t err)

/******************************************************************************
 *                          MACROS / DEFINITIONS / FLAGS
 *****************************************************************************/

// Set to print messages to assist with debugging.
#define SD_MSG 1 //   0  = NO messages
                 //   1  = ERROR messages
                 //   2  = INFO messages
                 //   3  = DEBUG messages
                 //  >3  = VERBOSE messages



// Host Capcity Support. Should always be set to 0. 
#define HCS 0  // 0 = host supports SDSC. 
               // 1 = host supports SDHC or SDXC.
              


// Data Block Length. Should always be 512.
#define DATA_BLOCK_LEN 512



// Macros for asserting / deasserting the (CS) pin.
#define CS_LOW    SPI_PORT &= ~(1<<SS);  // Assert CS
#define CS_HIGH   SPI_PORT |= (1<<SS);   // Deassert CS



// SD Card Commands available in SPI Mode.     
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



// R1 Response Flags (OUT_OF_IDLE and R1_TIMEOUT are not standard R1 responses)
#define OUT_OF_IDLE             0x00000   // No errors and out of idle state
#define IN_IDLE_STATE           0x00001
#define ERASE_RESET             0x00002
#define ILLEGAL_COMMAND         0x00004
#define COM_CRC_ERROR           0x00008
#define ERASE_SEQUENCE_ERROR    0x00010
#define ADDRESS_ERROR           0x00020
#define PARAMETER_ERROR         0x00040
#define R1_TIMEOUT              0x00080



// Responses returned by sd_SPI_Mode_Init() in addition to the R1 response.
#define FAILED_GO_IDLE_STATE    0x00100   //CMD0
#define FAILED_SEND_IF_COND     0x00200   //CMD8
#define UNSUPPORTED_CARD_TYPE   0x00400   //Card version and/or capacity
#define FAILED_CRC_ON_OFF       0x00800   //CMD59
#define FAILED_APP_CMD          0x01000   //CMD55
#define FAILED_SD_SEND_OP_COND  0x02000   //ACMD41
#define OUT_OF_IDLE_TIMEOUT     0x04000
#define FAILED_READ_OCR         0x08000   //CMD58
#define POWER_UP_NOT_COMPLETE   0x10000
#define FAILED_SET_BLOCKLEN     0x20000
#define FAILED_SEND_CSD         0x40000



/******************************************************************************
 *                           FUNCTION DECLARATIONS
******************************************************************************/



/******************************************************************************
 * Function:    sd_SPI_Mode_Init(void) 
 * Description: Initializes a standard capacity SD card into SPI mode
 * Argument(s): VOID
 * Returns:     uint32_t initialization response. The lowest byte of the 
 *              response is the last R1 response received. 
 *              Use sd_printInitResponse(response) to print the response.
******************************************************************************/
uint32_t sd_SPI_Mode_Init(void);



/******************************************************************************
 * Function:    sd_SendByte(uint8_t data)
 * Description: Send single byte to SD card via SPI.
 *              This, along with the sd_ReturnByte(), are the functions used to 
 *              interface directly with the SPI module.
 * Argument(s): uint8_t data byte to be sent to SD card.
 * Returns:     VOID
 * Notes:       Call this function as many times as necessary to send a command
 *              to the SD card in single byte packets.
*******************************************************************************/
void sd_SendByte(uint8_t data);



/******************************************************************************
 * Function:    sd_SendCommand(uint8_t cmd, uint32_t arg)
 * Description: sends SD command, argument, and CRC via SPI.
 * Argument(s): 8-bit SD card command from the command list.
 *              32-bit argument.
 * Returns:     VOID
 * Notes:       sd_CRC7 is called in this function to calculate the CRC7 value
 *              to send with the corresponding Command/Argument combination.
******************************************************************************/
void sd_SendCommand(uint8_t cmd, uint32_t arg);



/******************************************************************************
 * Function:    sd_ReturnByte(void)
 * Description: Get byte returned by SD card via SPI.
 *              This, along with the sd_SendByte(), are the only functions that
 *              explicitly interface with the SPI specific functions.
 * Argument(s): VOID
 * Returns:     8-bit SD Card response.
 * Notes:       1) If a multi-byte value is expected to be returned then this
 *                 function should be called at least that many times.
 *              2) This function will return whatever byte value is present in 
 *                 the SPDR when the SPIF flag is set in SPI_MasterRead(). It 
 *                 is up to the calling function to determine if the returned 
 *                 value is valid.
******************************************************************************/
uint8_t sd_ReturnByte(void);



/******************************************************************************
 * Function:    sd_CRC7(uint64_t tca)
 * Description: Generates and returns CRC7 bits for SD command/argument 
 *              combination. Should only be called from sd_SendCommand()
 * Argument(s): 40-bit Transmission, Command, Argument (tca) bits to be sent as
 *              command to SD Card. 24-bit leading zeros in the argument are
 *              not used
 * Returns:     1 byte with the 7 most significant bits corresponding to the 
 *              calculated CRC7.  
 * Notes:       The value of the LSB returned does not matter, it will be 
 *              set to 1 as the transmission stop bit regardless of the value
 *              returned here.
******************************************************************************/
uint8_t sd_CRC7(uint64_t ca);



/******************************************************************************
 * Function:    sd_getR1(void)
 * Description: Gets the R1 response returned by an SD card for a given command.
 * Argument(s): VOID
 * Returns:     uint8_t byte corresponding to the R1 response.
 * Notes:       The R1 response is the first response byte sent by an SD Card
 *              in response to any command, therefore, this function should 
 *              not be called at any other time except to get the first byte 
 *              response, otherwise the returned value will not be the R1
 *              response of the SD Card.
******************************************************************************/
uint8_t sd_getR1(void);



/******************************************************************************
 * Function:    sd_printR1(uint8_t R1)
 * Description: Prints the R1 response in readable form.
 * Argument(s): uint8_t R1 response
 * Returns:     VOID
 * Notes:       The true SD card SPI mode R1 response only occupies bits 0 to 
 *              7 of the first byte returned by the SD card in response to any
 *              command. In this implementation, bit 8 is used as a R1_TIMEOUT
 *              flag and will be set to 1 if it takes too long for the SD card
 *              to return the R1 response
******************************************************************************/
void sd_printR1(uint8_t R1);



/******************************************************************************
 * Function:    sd_printInitResponse(uint32_t err)
 * Description: Prints the response returned by sd_SPI_Mode_Init() in a
 *              readable form.
 * Argument(s): 32-bit response of sd_SPI_Mode_Init()
 * Returns:     VOID
 * Notes:       The sd_SPI_Mode_Init() response includes the last R1 response 
 *              returned by the SD card as well as the other initialization 
 *              errors.       
 * ***************************************************************************/
void sd_printInitResponse(uint32_t err);



Base-Level Functions in SD_SPI.C:
  1) sd_SPI_Mode_Init(): An SD initialization routine to intialize the SD card into SPI mode. This    must be must be called first and complete successfully before any other interaction with the SD card will be successful/valid. Returns an initialization error code that also includes the most recent R1 response.  An initialization error code of 0 indicates the card has been successfully initialized. Any other error code indicates the initialization failed. Pass the error to sd_printInitResponse(err) to read the initialization error.
  2) sd_SendCommand(cmd, arg): Sends the command/argument/CRC7 combination to the SD card via the SPI port.  The CRC7 value is calculated and returned from sd_CRC7() which is called directly from the sd_SendCommand function.
  3) sd_Response(): Waits for, and returns the SD card's response for a given command. Only a single byte (8-bit) response will be returned each time this function is called so call this function must be called as many times as necessary to read in the full response to a command.

Helper Functions in SD_SPI.C:
  4) sd_CRC7(): calculates the CRC 7-bit value for a given command/arguement combination.  This function is called by sd_SendCommand which will send the CRC7 bit as part of the command, whether it is needed or not. By default CRC check is turned off for an SD Card in SPI mode, but it is required through command 8 (SEND_IF_COND) of the initializatin routine.
  5) sd_getR1(): routine to simplify getting the R1 SD card response to any command.
  6) sd_printR1(R1_response): interprets and prints to screen the R1 response returned for a give command in a human-readable form.
  6) sd_printInitResponse(): interprets and prints to screen the error code value returned by the intialization routine.  The initialization error code includes the most recent R1 response and thus sd_printR1() is also called from this function to print the R1 response portion of the initialization error.


Once the SD Card is initialized, all other host interaction with the SD Card can be performed by calling the sd_Command() and sd_Response() functions.  The response must be interpreted and handled by the calling function as a single call to sd_Response will only return an 8-bit value, if the response if longer than 8-bits then sd_Response will need to be called repeatedly until the complete response has been read in.

The SD_MSG flag in SD_SPI.H can be used for printing messages associated with functions in SD_SPI.C.  This was helpful when writing the code so I left it in.  If you use this code, you may find it useful as well while troubleshooting.  Recommend setting this flag to 1 for normal operation (ERROR messages only).

An SDMISC.C is intended to be for any additional miscellaneous SD Card raw data access, calculation, and printing functions that I decide to create.


# Additional Notes / Limitations / Warnings
* This code has only been tested using an ATmega1280 microcontroller, but assuming program data space and memory are sufficient, I would expect it to be easily ported to other comparable AVR microcontrollers once the USART and SPI port assignments are modified.
* Currently the code has only been tested with a 2GB SD card and thus only supports standard capacity SD cards (SDSC).

# Who can use
Anyone can use this and modify it to meet the needs of their system, but let me know if you use it or find it helpful.

# Requirements
[AVR Toolchain](https://github.com/osx-cross/homebrew-avr)

# Reference Documents:
1) Atmel ATmega640/V-1280/V-1281/V-2560/V-2561/V Datasheet
2) SD Specifications Part 1: Physical Layer Specification - Simplified Specification Version 7.10
