/******************************************************************************
 * SD_SPI_BASE.C
 * 
 * TARGET
 * ATmega 1280
 *
 * DESCRIPTION
 * Defines the base-level SD Card functions declared in SD_SPI.H
 * 
 * FUNCTION LIST
 * 1) uint32_t  sd_SPI_Mode_Init(void)
 * 2) void      sd_SendByte(uint8_t data)
 * 3) uint8_t   sd_ReturnByte(void)
 * 4) void      sd_SendCommand(uint8_t cmd, uint32_t arg)
 * 5) uint8_t   sd_CRC7(uint64_t tca)
 * 6) uint8_t   sd_getR1(void) 
 * 7) void      sd_printR1(uint8_t R1)
 * 8) void      sd_printInitResponse(uint32_t err)
 * 
 * Author: Joshua Fain
 * Date:   9/17/2020
 * ***************************************************************************/
 

#include <stdint.h>
#include <avr/io.h>
#include "../includes/sd_spi_base.h"
#include "../../../../general/includes/spi.h"
#include "../../../../general/includes/prints.h"



// Initializes a standard capacity SD card into SPI mode
uint32_t sd_SPI_Mode_Init(void)
{
    if(SD_MSG > 1) print_str("\n\n\r>> INFO:    ***** BEGIN sd_SPI_Mode_Init()");
    
    //Initialize response variables
    uint8_t R1 = 0; //R1 response returned for every SD Command 
    uint8_t R7[5]; //R7 response returned by SEND_IF_COND (CMD8)

    for(int i=0;i<=10;i++)  sd_SendByte(0xFF); //Wait 80 clock cycles for power up to complete.

    // ********************
    // GO_IDLE_STATE (CMD0) : place card in SPI mode  
    CS_LOW; // Assert CS          
    sd_SendCommand(GO_IDLE_STATE, 0);  //CMD0;
    R1 = sd_getR1(); // Get R1 response
    CS_HIGH; // Deassert CS
    if(SD_MSG > 2) {print_str("\n\r>> DEBUG:   Printing R1 response returned from sd_getR1() for command GO_IDLE_STATE (CMD0):"); sd_printR1(R1);}
    if (R1 != 1)
    {
        if(SD_MSG) 
        {
            print_str("\n\r>> ERROR:   GO_IDLE_STATE (CMD0) in sd_SPI_Mode_Init() returned error in R1 response.");
            print_str("\n\r>> ERROR:   Initialization FAILED. Returning from sd_SPI_Mode_Init() with error(s).");
        }
        return (FAILED_GO_IDLE_STATE | R1);
    }
    //END GO_IDLE_STATE (CMD0)
    // *******************



    // *******************
    // SEND_IF_COND (CMD8): get SD Interface Conditions. Confirm card type is valid/supported.
    uint8_t cp = 0xAA; //check pattern
    uint8_t v  = 0x01; //supply voltage: range = 2.7 to 3.6V
    
    CS_LOW;
    sd_SendCommand(SEND_IF_COND, ((uint16_t)v<<8)|cp); //CMD8

    //Get R7 response
    R7[0] = sd_getR1(); // First byte of R7 response is the R1 response.
    R7[1] = sd_ReturnByte();
    R7[2] = sd_ReturnByte();
    R7[3] = sd_ReturnByte();
    R7[4] = sd_ReturnByte();
    
    CS_HIGH;

    if(SD_MSG > 2) {print_str("\n\r>> DEBUG:   Printing R1 response returned from sd_getR1() for command SEND_IF_COND (CMD8):"); sd_printR1(R7[0]);}
    if (R7[0] != 1) //R7[0] is R1 response returned for CMD8
    {
        if(SD_MSG) 
        {
            print_str("\n\r>> ERROR:   SEND_IF_COND (CMD8) in sd_SPI_Mode_Init() returned error in R1 response.");
            print_str("\n\r>> ERROR:   Initialization FAILED. Returning from sd_SPI_Mode_Init() with error(s).");
        }
        return (FAILED_SEND_IF_COND | R7[0]);
    }

    if(R7[3]!=0x01) //Check the rest of the response to confirm if SEND_IF_COND is valid.
    {
        if(SD_MSG)
        {
            print_str("\n\r>> ERROR:   Voltage mismatch detected in SD Card's response to SEND_IF_COND (CMD8) in sd_SPI_Mode_Init().");
            print_str("\n\r            Either SD card version is invalid/unsupported OR the 'supply voltage' portion of the argument in SEND_IF_COND command is incorrect. ");
            print_str("\n\r            Ensure 'supply voltage' argument = 0x01.");
            print_str("\n\r            Ensure SD Card is version 2.0 or later.");
            print_str("\n\r>> ERROR:   Initialization FAILED. Returning from sd_SPI_Mode_Init() with error(s).");
        }
        return (FAILED_SEND_IF_COND | UNSUPPORTED_CARD_TYPE | R7[0]);
    }
    if(R7[4]!=cp)
    {
        if(SD_MSG)
        {
            print_str("\n\r>> ERROR:   Check Pattern failed in SD Card's response to SEND_IF_COND (CMD8) in sd_SPI_Mode_Init().");
            print_str("\n\r            Check pattern sent with SEND_IF_COND command to SD Card:");
            print_str(" 0x");print_hex(cp);
            print_str("\n\r            Check pattern returned by SD Card:");
            print_str(" 0x");print_hex(R7[4]);
            print_str("\n\r>> ERROR:   Initialization FAILED. Returning from sd_SPI_Mode_Init() with error(s).");
        }
        return (FAILED_SEND_IF_COND | UNSUPPORTED_CARD_TYPE | R7[0]);
    }
    //END SEND_IF_COND (CMD8)
    // **********************



    // **********************
    // CRC_ON_OFF (CMD59): turns CRC ON or OFF. Should be off by default, 
    R1 = 0; // Reset R1
    CS_LOW;
    sd_SendCommand(CRC_ON_OFF,0);  //Send CMD 59. 
                                   //arg = 0 CRC OFF (default for SPI mode). 
                                   //arg = 1 CRC ON 
    R1 = sd_getR1(); // Get R1 response
    CS_HIGH;
    if(SD_MSG > 2) {print_str("\n\r>> DEBUG:   Printing R1 response returned from sd_getR1() for command CRC_ON_OFF (CMD59):"); sd_printR1(R1);}
    if (R1 != 1)
    {
        if(SD_MSG) 
        {
            print_str("\n\r>> ERROR:   CRC_ON_OFF (CMD59) in sd_SPI_Mode_Init() returned error in R1 response.");
            print_str("\n\r>> ERROR:   Initialization FAILED. Returning from sd_SPI_Mode_Init() with error(s).");
        }
        return (FAILED_CRC_ON_OFF | R1);
    }
    //END CRC_ON_OFF (CMD59)
    // ***********************



    // ************************
    // SD_SEND_OP_COND (ACMD41): 
    // Send host capacity information to SD card. Continue sending until SD card signals it is no 
    // longer in the idle state (R1 != 1). SD_SEND_OP_COND is an application command therefore each
    // time it is sent it must be preceded by APP_CMD (CMD55) to signal the next command is ACMD type.
    uint32_t acmd41_arg = 0;
    if(HCS) // acmd41_arg = 0x40000000 if HCS supported;
    {
        if(SD_MSG)
        {
            print_str("\n\r>> ERROR:   HCS flag is > 0 indicating host should support SDHC/SDXC cards,"); 
            print_str("\n\r            SDHC/SDXC cards are currently not supported by this host.");
            print_str("\n\r            Set HCS flag to 0 and recompile."); 
            print_str("\n\r>> ERROR:   Initialization FAILED. Returning from sd_SPI_Mode_Init() with error(s).");
        }
        return (UNSUPPORTED_CARD_TYPE | R1);
    }

    else
    {
        uint8_t attempt = 0; //used for timeout returning OUT_OF_IDLE
        do{
            R1 = 0; // Reset R1
            CS_LOW;
            sd_SendCommand(APP_CMD,0); //send APP_CMD (CMD55) to signal next command is ACMD type.
            R1 = sd_getR1();
            CS_HIGH;
            if(SD_MSG > 2) {print_str("\n\r>> DEBUG:   Printing R1 response returned from sd_getR1() for command APP_CMD (CMD55):"); sd_printR1(R1);}
            if (R1 != 1)
            {
                if(SD_MSG)
                {
                    print_str("\n\r>> ERROR:   APP_CMD (CMD55) in sd_SPI_Mode_Init() returned error in R1 response.");
                    print_str("\n\r>> ERROR:   Initialization FAILED. Returning from sd_SPI_Mode_Init() with error(s).");
                }
                return (FAILED_APP_CMD | R1);
            }
            
            R1 = 0; // Reset R1
            CS_LOW;
            sd_SendCommand(SD_SEND_OP_COND,acmd41_arg); //send SD_SEND_OP_COND (ACMD41)
            R1 = sd_getR1();
            CS_HIGH;
            if(SD_MSG > 2) {print_str("\n\r>> DEBUG:   Printing R1 response returned from sd_getR1() for command SD_SEND_OP_COND (ACMD41):"); sd_printR1(R1);}
            if (R1 > 1)
            {
                if(SD_MSG) 
                { 
                    print_str("\n\r>> ERROR:   SD_SEND_OP_COND (ACMD41) in sd_SPI_Mode_Init() returned error in R1 response.");
                    print_str("\n\r>> ERROR:   Initialization FAILED. Returning from sd_SPI_Mode_Init() with error(s).");
                }
                return (FAILED_SD_SEND_OP_COND | R1);
            }

            if(attempt++ >= 0x05)
            {
                if(SD_MSG) 
                {
                    print_str("\n\r>> ERROR:   Time out while waiting for R1 response to SD_SEND_OP_COND to signal SD card is out of the idle state in sd_SPI_Mode_Init().");
                    print_str("\n\r>> ERROR:   Initialization FAILED. Returning from sd_SPI_Mode_Init() with error(s).");
                }
                return (FAILED_SD_SEND_OP_COND | OUT_OF_IDLE_TIMEOUT | R1);
            }
        }while(R1&1);
    }
    // END SEND_OP_COND (ACMD41)
    // ************************



    // ************************
    // READ_OCR (CMD58): 
    // Final step for proper initialization is to read CCS (Card Capactity Status) bit
    // of the OCR (Operating Condition Register). Currently only CCS = 0 is supported on host.
    uint8_t resp;
    R1 = 0; // Reset R1
    CS_LOW;
    sd_SendCommand(READ_OCR,0);  //Send CMD 58
    R1 = sd_getR1();
    if(SD_MSG > 2) {print_str("\n\r>> DEBUG:   Printing R1 response returned from sd_getR1() for command READ_OCR (CMD58):"); sd_printR1(R1);}
    if (R1 != 0)
    {
        if(SD_MSG)
        {
            print_str("\n\r>> ERROR:   READ_OCR (CMD58) in sd_SPI_Mode_Init() returned error in R1 response.");
            print_str("\n\r>> ERROR:   Initialization FAILED. Returning from sd_SPI_Mode_Init() with error(s).");
        }
        return (FAILED_READ_OCR | R1);
    }

    resp = sd_ReturnByte();  //Get the rest of the response to READ_OCR

    //Check POWER_UP_STATUS
    if((resp>>7)!=1)
    {
        CS_HIGH;
        if(SD_MSG)
        {
            print_str("\n\r>> ERROR:   POWER_UP_STATUS bit in OCR is 1. SD Card has not completed power up cycle.");
            print_str("\n\r>> ERROR:   Initialization FAILED. Returning from sd_SPI_Mode_Init() with error(s).");
        }
        return (POWER_UP_NOT_COMPLETE | R1);
    }
    
    //Check rest of the OCR. Only CCS bit is needed to complete initialization of SD Card in SPI Mode.
    uint8_t CCS  = ((resp&0x40)>>6);
    uint8_t UHSII = ((resp&0x20)>>5);
    uint8_t CO2T = ((resp&0x10)>>3);
    uint8_t S18A = (resp&0x01);
    uint16_t VOLTAGE_RANGES_ACCEPTED = sd_ReturnByte();
             VOLTAGE_RANGES_ACCEPTED <<= 1;
             VOLTAGE_RANGES_ACCEPTED |= (sd_ReturnByte()>>7);


    if(CCS > 0)
    {
        CS_HIGH;
        if(SD_MSG)
        {
            print_str("\n\r>> ERROR:   CCS bit of OCR is = 1 and is not currently supported by host.");
            print_str("\n\r>> ERROR:   Initialization FAILED. Returning from sd_SPI_Mode_Init() with error(s).");
        }
        return (FAILED_READ_OCR | UNSUPPORTED_CARD_TYPE | R1) ;
    }

    if( (UHSII !=  0) || //0 for non-UHSII
        (CO2T  !=  0) || //0 for Over 2TB NOT supported
        (S18A  !=  0) || //0 for NO switching to 1.8V Accepted.
        (VOLTAGE_RANGES_ACCEPTED != 0x1FF)  )//Voltages over range 2.7 to 3.6V supported.
    {
        CS_HIGH;
        if(SD_MSG)
        {
            print_str("\n\r>> ERROR:   Unexpected response to READ_OCR in sd_SPI_Mode_Init() for UHSII, CO2T, S18A, or VOLTAGE_RANGES.");
            print_str("\n\r>> ERROR:   Initialization FAILED. Returning from sd_SPI_Mode_Init() with error(s).");
        }
        return (FAILED_READ_OCR | UNSUPPORTED_CARD_TYPE | R1);
    }
    CS_HIGH;
    // END READ_OCR (CMD58)
    // ************************

    if(SD_MSG > 1) print_str("\n\r>> INFO:    ***** END sd_SPI_Mode_Init(): SD Card successfully initialized.");
    return OUT_OF_IDLE; //initialization succeded

}
//END sd_SPI_Mode_Init()



// Sends single byte argument to SD card via SPI.
void sd_SendByte(uint8_t data)
{
    SPI_MasterTransmit(data);
}
//END sd_SendByte()



// Gets a single byte response return by SD card. If a multi-byte value is
// expected, then call enough times to read in the full response.
uint8_t sd_ReturnByte(void)
{
    sd_SendByte(0xFF);
    return SPI_MasterRead();
} 
//END sd_ReturnByte()



// Sends SD command, argument, and CRC via SPI.
void sd_SendCommand(uint8_t cmd, uint32_t arg)
{
    uint8_t tc = 0x40 | cmd;    //Initialize transmit and command byte. 
                                //The two MSBs of command are always 0b01
    
    //Initialize variable for 48-bit SD Card command. From MSB to LSB this is 
    //TRANSMIT (2b) | CMD (6b) | ARG (32b) | CRC (7b) | STOP (1b)
    uint64_t tcacs = 0;

    //shift TRANSMIT, CMD, and ARG bits into place
    tcacs = (tcacs | tc) << 40;
    tcacs = tcacs | ((uint64_t)(arg) << 8);

    uint8_t crc = sd_CRC7(tcacs); //Get CRC for command
 
    tcacs = tcacs | crc | 1;  //complete loading of 48-bit command into tcacs
                              //by setting CRC and stop transmission bits

    if(SD_MSG > 2) 
    {
        if(cmd==41) print_str("\n\r>> DEBUG:   SENDING [ACMD "); 
        else print_str("\n\r>> DEBUG:   SENDING [CMD "); 
        print_dec(cmd);  
        print_str(" | ARG 0x"); print_hex(arg);  
        print_str(" | CRC 0x"); print_hex(crc);
        print_str("]");
    }

    if(SD_MSG > 3)
    {
        print_str("\n\r>> VERBOSE: Bytes sent for SD Command:");
        print_str("\n\r            Byte[5]: 0x"); print_hex((uint8_t)(tcacs >> 40));
        print_str("\n\r            Byte[4]: 0x"); print_hex((uint8_t)(tcacs >> 32));
        print_str("\n\r            Byte[3]: 0x"); print_hex((uint8_t)(tcacs >> 24));
        print_str("\n\r            Byte[2]: 0x"); print_hex((uint8_t)(tcacs >> 16));
        print_str("\n\r            Byte[1]: 0x"); print_hex((uint8_t)(tcacs >> 8));
        print_str("\n\r            Byte[0]: 0x"); print_hex((uint8_t)tcacs);
    }

    for(int i=0;i<=2;i++)   sd_SendByte(0xFF);	//Wait 16 clock cycles before sending command.
    
    // Send command to SD Card via SPI
    sd_SendByte((uint8_t)(tcacs >> 40));
    sd_SendByte((uint8_t)(tcacs >> 32));
    sd_SendByte((uint8_t)(tcacs >> 24));
    sd_SendByte((uint8_t)(tcacs >> 16));
    sd_SendByte((uint8_t)(tcacs >> 8));
    sd_SendByte((uint8_t)tcacs);
} 
//END  sd_SendCommand()



// Generates and returns CRC7 bits for SD command/argument
uint8_t sd_CRC7(uint64_t tca)
{    
    uint64_t test = 0x800000000000; //initialize test variable used to 
                                    //determine if division will take place 
                                    //during a given iteration.
    
    //divisor 0b10001001 (0x89) used by SD standard to generate CRC7. 
    //byte 6 of divisor variable is initialized with this value.
    uint64_t divisor = 0x890000000000;

    //initialize result with transmit/command/argument portion of SD command.
    uint64_t result = tca; 

    //calculate CRC7
    for (int i = 0; i < 40; i++)
    {
        if(result&test)
            result ^= divisor;        
        divisor  = (divisor >> 1);
        test = (test >> 1);        
    }
    return(result);
} 
//END _CRC7()



// Gets the R1 response returned by an SD card for a given command.
uint8_t sd_getR1(void)
{
    uint8_t R1;
    uint8_t attempt = 0;

    while((R1 = sd_ReturnByte()) == 0xFF)
    {  
        if(attempt++ >= 0xFF)
            return R1_TIMEOUT;
    }
    return R1;
}
//END sd_getR1()



// Prints the results of the R1 response in readable form.
void sd_printR1(uint8_t R1)
{
    if(SD_MSG)  // modified 9/1/2020
    {
        if(R1&R1_TIMEOUT)
            print_str(" R1_TIMEOUT,"); //Not part of SD card R1 standard.
        if(R1&PARAMETER_ERROR)
            print_str(" PARAMETER ERROR,");
        if(R1&ADDRESS_ERROR)
            print_str(" ADDRESS ERROR,");
        if(R1&ERASE_SEQUENCE_ERROR)
            print_str(" ERASE SEQUENCE ERROR");
        if(R1&COM_CRC_ERROR)
            print_str(" COM_CRC_ERROR");
        if(R1&ILLEGAL_COMMAND)
            print_str(" ILLEGAL COMMAND");
        if(R1&ERASE_RESET)
            print_str(" ERASE RESET");
        if(R1&IN_IDLE_STATE)
            print_str(" IN IDLE STATE");
        if(R1==0) // R1 = 0 NO ERRORS;
            print_str(" OUT OF IDLE");
    }
}
//END sd_printR1()



// Prints the response returned by sd_SPI_Mode_Init() in a 
void sd_printInitResponse(uint32_t err)
{
    if(SD_MSG)
    {
        //print R1 portion of initiailzation response
        print_str("\n\r>> R1 response returned from SD Card Initialization:");
        sd_printR1((uint8_t)(0x000FF&err));

        print_str("\n\r>> Initialization response returned from SD Card Initialization:");
        //print other portion of R1 response
        if(err&FAILED_GO_IDLE_STATE)
            print_str(" FAILED_GO_IDLE_STATE\n\r");
        if(err&FAILED_SEND_IF_COND)
            print_str(" FAILED_SEND_IF_COND");
        if(err&UNSUPPORTED_CARD_TYPE)
            print_str(" UNSUPPORTED_CARD_TYPE");
        if(err&FAILED_CRC_ON_OFF)
            print_str(" FAILED_CRC_ON_OFF");
        if(err&FAILED_APP_CMD)
            print_str(" FAILED_APP_CMD");
        if(err&FAILED_SD_SEND_OP_COND)
            print_str(" FAILED_SD_SEND_OP_COND");
        if(err&OUT_OF_IDLE_TIMEOUT)
            print_str(" OUT_OF_IDLE_TIMEOUT");
        if(err&FAILED_READ_OCR)
            print_str(" FAILED_READ_OCR");
        if(err&POWER_UP_NOT_COMPLETE)
            print_str(" POWER_UP_NOT_COMPLETE");
        if(err&FAILED_SET_BLOCKLEN)
            print_str(" FAILED_SET_BLOCKLEN");
        if(err&FAILED_SEND_CSD)
            print_str(" FAILED_SEND_CSD");
        if(err == 0) // NO ERRORS
            print_str(" INIT_SUCCESS\n\r");
    }
}
//END sd_printInitErrors()
