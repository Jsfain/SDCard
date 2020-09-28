/******************************************************************************
 * SD_SPI_BASE.H
 *  
 * TARGET
 * ATmega 1280
 *
 * DESCRIPTION
 * Base-level SD card function declarations and struct and macro definitions  
 * that will be used to handle the basic physical interaction with an SD card
 * operating in SPI Mode.
 * 
 * Author: Joshua Fain
 * Date:   9/24/2020
 * ***************************************************************************/



#ifndef SD_SPI_H
#define SD_SPI_H



/******************************************************************************
 *                         DEFINE MACROS / FLAGS
 *****************************************************************************/


// Host Capcity Support.
#define HCS 1  // 0 = host only supports SDSC. 
               // 1 = host also supports SDHC or SDXC.
              
// Block Length. Currently only supports values of 512.
#define BLOCK_LEN 512

// Asserting / deasserting the (CS) pin.
#define CS_LOW    SPI_PORT &= ~(1<<SS);  // Assert
#define CS_HIGH   SPI_PORT |= (1<<SS);   // Deassert



// SD card commands available in SPI Mode.     
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



// R1 Response Flags. 
// OUT_OF_IDLE and R1_TIMEOUT are not part of the SD card's R1
// responses, but are used here to indicate additional states. 
#define OUT_OF_IDLE             0x00000   // No errors & out of idle
#define IN_IDLE_STATE           0x00001
#define ERASE_RESET             0x00002
#define ILLEGAL_COMMAND         0x00004
#define COM_CRC_ERROR           0x00008
#define ERASE_SEQUENCE_ERROR    0x00010
#define ADDRESS_ERROR           0x00020
#define PARAMETER_ERROR         0x00040
#define R1_TIMEOUT              0x00080



// Initialization Error Responses returned by initialization function.
// The lowest byte is zero to accommodate the R1 response.
#define FAILED_GO_IDLE_STATE    0x00100   //CMD0
#define FAILED_SEND_IF_COND     0x00200   //CMD8
#define UNSUPPORTED_CARD_TYPE   0x00400
#define FAILED_CRC_ON_OFF       0x00800   //CMD59
#define FAILED_APP_CMD          0x01000   //CMD55
#define FAILED_SD_SEND_OP_COND  0x02000   //ACMD41
#define OUT_OF_IDLE_TIMEOUT     0x04000
#define FAILED_READ_OCR         0x08000   //CMD58
#define POWER_UP_NOT_COMPLETE   0x10000



// Card Types
#define SDHC 1 // or SDXC
#define SDSC 0 // standard capacity


// THe SD card's version and type will be held in an object of this struct.
// The members should only be set by initialization routine.
typedef struct {
    uint8_t version;
    uint8_t type;
} CardTypeVersion;


/******************************************************************************
 *                           FUNCTION DECLARATIONS
******************************************************************************/


// Initializes the SD card into SPI mode and gets the card type and version.
uint32_t SD_InitializeSPImode(CardTypeVersion * ctv);

// Sends single byte argument to SD card via SPI.
void SD_SendByteSPI(uint8_t byte);

// Gets a single byte returned by the SD card.
uint8_t SD_ReceiveByteSPI(void);

// Sends SD command/argument/CRC via SPI.
void SD_SendCommand(uint8_t cmd, uint32_t arg);

// Gets the R1 response returned by an SD card for a given command.
uint8_t SD_GetR1(void);

// Prints the results of the R1 response in readable form.
void SD_PrintR1(uint8_t r1);

// Prints the initialization error response 
// returned by SD_InitializeSPImode().
void SD_PrintInitError(uint32_t err);


#endif //SD_SPI_H