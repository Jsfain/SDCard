/******************************************************************************
 * SD_SPI_BASE.C
 *
 * TARGET
 * ATmega 1280
 *
 * DESCRIPTION
 * Defines the base-level SD card functions for SPI mode.
 *
 * FUNCTION LIST
 * 1) uint32_t  SD_InitializeSPImode(void)
 * 2) void      SD_SendByteSPI(uint8_t byte)
 * 3) uint8_t   SD_ReceiveByteSPI(void)
 * 4) void      SD_SendCommand(uint8_t cmd, uint32_t arg)
 * 5) uint8_t   SD_GetR1(void)
 * 6) void      SD_PrintR1(uint8_t r1)
 * 7) void      SD_PrintInitError(uint32_t err)
 *
 * Author: Joshua Fain
 * Date:   9/20/2020
 * ***************************************************************************/



#include <stdint.h>
#include <avr/io.h>
#include "../includes/sd_spi_base.h"
#include "../../../../general/includes/spi.h"
#include "../../../../general/includes/prints.h"



/******************************************************************************
 *                        "PRIVATE" FUNCTION DECLARATIONS
******************************************************************************/

uint8_t pvt_CRC7(uint64_t ca);



/******************************************************************************
 *                         "PUBLIC" FUNCTION DEFINITIONS
******************************************************************************/

// Initializes a standard capacity SD card into SPI mode
uint32_t SD_InitializeSPImode(void)
{
    uint8_t r1 = 0; //R1 response returned for every SD Command
    uint8_t R7[5];  //R7 response returned by SEND_IF_COND (CMD8)

    //Wait up to 80 clock cycles for power up to complete.
    for(int i=0;i<=10;i++) { SD_SendByteSPI(0xFF); }

    // ********************
    // GO_IDLE_STATE (CMD0)
    CS_LOW;
    SD_SendCommand(GO_IDLE_STATE, 0);
    r1 = SD_GetR1();
    CS_HIGH;
    if (r1 != 1) { return (FAILED_GO_IDLE_STATE | r1); }
    // END GO_IDLE_STATE (CMD0)
    // *******************



    // *******************
    // SEND_IF_COND (CMD8)
    uint8_t checkPattern = 0xAA;
    uint8_t voltageSupplyRange  = 0x01; // 2.7 to 3.6V
    
    CS_LOW;
    SD_SendCommand(SEND_IF_COND,
                  ((uint16_t)voltageSupplyRange << 8 ) | checkPattern );

    //Get R7 response
    R7[0] = SD_GetR1(); // First R7 byte is R1 response.
    R7[1] = SD_ReceiveByteSPI();
    R7[2] = SD_ReceiveByteSPI();
    R7[3] = SD_ReceiveByteSPI();
    R7[4] = SD_ReceiveByteSPI();
    
    CS_HIGH;
    if (R7[0] != 1) { return (FAILED_SEND_IF_COND | R7[0]); }

    if ( (R7[3] != voltageSupplyRange ) || (R7[4] != checkPattern) )
    { return (FAILED_SEND_IF_COND | UNSUPPORTED_CARD_TYPE | R7[0]); }
    // END SEND_IF_COND (CMD8)
    // **********************



    // **********************
    // CRC_ON_OFF (CMD59)
    r1 = 0;
    CS_LOW;
    SD_SendCommand(CRC_ON_OFF,0);  //arg = 0 CRC OFF (default)
                                   //arg = 1 CRC ON
    r1 = SD_GetR1();
    CS_HIGH;
    if (r1 != 1) { return (FAILED_CRC_ON_OFF | r1); }
    // END CRC_ON_OFF (CMD59)
    // ***********************



    // ************************
    // SD_SEND_OP_COND (ACMD41)
    uint32_t acmd41_arg = 0; // = 0x40000000 if HCS supported;
    if(HCS) { return (UNSUPPORTED_CARD_TYPE | r1); } // HCS is not supported
    else
    {
        // Continue sending host capacity info to SD card until card signals it
        // is no longer in the idle state (r1 != 1) or times out.
        uint8_t timeout = 0;
        do{
            r1 = 0;
            CS_LOW;
            SD_SendCommand(APP_CMD,0); //first send APP_CMD before any ACMD
            r1 = SD_GetR1();
            CS_HIGH;
            if (r1 != 1) { return (FAILED_APP_CMD | r1); }

            r1 = 0;
            CS_LOW;
            SD_SendCommand(SD_SEND_OP_COND,acmd41_arg);
            r1 = SD_GetR1();
            CS_HIGH;
            if (r1 > 1) { return (FAILED_SD_SEND_OP_COND | r1); }

            if(timeout++ >= 0x05)
            { return (FAILED_SD_SEND_OP_COND | OUT_OF_IDLE_TIMEOUT | r1); }
        }while( r1 & 1 );
    }
    // END SEND_OP_COND (ACMD41)
    // ************************



    // ************************
    // READ_OCR (CMD 58)

    // Final initialization step is to read CCS (Card Capactity Status)
    // bit of the OCR (Operating Condition Register).
    r1 = 0;
    CS_LOW;
    SD_SendCommand(READ_OCR,0);
    r1 = SD_GetR1();
    if (r1 != 0) { return (FAILED_READ_OCR | r1); }

    uint8_t resp = SD_ReceiveByteSPI();  //Get the rest of the response to READ_OCR

    //POWER_UP_STATUS
    if( (resp >> 7) != 1 )
    {
        CS_HIGH;
        return (POWER_UP_NOT_COMPLETE | r1);
    }
    
    // Confirm rest of OCR returned is valid, though, technically
    // only CCS is needed to complete initialization.
    uint8_t ccs   = ((resp&0x40)>>6);
    uint8_t uhsii = ((resp&0x20)>>5);
    uint8_t co2t  = ((resp&0x10)>>3);
    uint8_t s18a  = (resp&0x01);
    uint16_t voltagRangesAccepted = SD_ReceiveByteSPI();
             voltagRangesAccepted <<= 1;
             voltagRangesAccepted |= (SD_ReceiveByteSPI()>>7);

    if(ccs > 0) // currently only non-HCS supported.
    {
        CS_HIGH;
        return (FAILED_READ_OCR | UNSUPPORTED_CARD_TYPE | r1) ;
    }

    if( (uhsii !=  0) || //0 - only non-UHSII supported
        (co2t  !=  0) || //0 - Over 2TB NOT supported
        (s18a  !=  0) || //0 - NO switching to 1.8V Accepted.
        (voltagRangesAccepted != 0x1FF)  ) // Voltage range of
                                           // 2.7 to 3.6V supported.
    {
        CS_HIGH;
        return (FAILED_READ_OCR | UNSUPPORTED_CARD_TYPE | r1);
    }
    CS_HIGH;
    // END READ_OCR (CMD58)
    // ************************

    return OUT_OF_IDLE; //initialization succeded
}
// END SD_InitializeSPImode()



// Sends single byte argument to SD card via SPI.
void SD_SendByteSPI(uint8_t byte)
{
    SPI_MasterTransmit(byte);
}
// END SD_SendByteSPI()



// Gets a single byte response return by SD card. If a multi-byte value is
// expected, then call enough times to read in the full response.
uint8_t SD_ReceiveByteSPI(void)
{
    SD_SendByteSPI(0xFF);
    return SPI_MasterRead();
}
// END SD_ReceiveByteSPI()



// Sends SD command, argument, and CRC via SPI.
void SD_SendCommand(uint8_t cmd, uint32_t arg)
{
    // Structure of command / argument sent to an
    // SD card via SPI, from MSB --> LSB. b = bit.
    // TRANSMIT (2b) = 0b01 | CMD (6b) | ARG (32b) | CRC (7b) | STOP (1b) = 0b1
    uint64_t tcacs = 0;

    uint8_t tc = 0x40 | cmd; // Transmit & Command byte.
                             // Transmit bits are always 0b01
    
    tcacs = (tcacs | tc) << 40;
    tcacs = tcacs | ((uint64_t)(arg) << 8);

    uint8_t crc = pvt_CRC7(tcacs);
 
    tcacs = tcacs | crc | 1;  //complete loading of 48-bit command into tcacs
                              //by setting CRC and stop transmission bits

    for(int i=0;i<=2;i++) { SD_SendByteSPI(0xFF); } // Wait 16 clk cycles.
    
    // Send command to SD Card via SPI
    SD_SendByteSPI((uint8_t)(tcacs >> 40));
    SD_SendByteSPI((uint8_t)(tcacs >> 32));
    SD_SendByteSPI((uint8_t)(tcacs >> 24));
    SD_SendByteSPI((uint8_t)(tcacs >> 16));
    SD_SendByteSPI((uint8_t)(tcacs >> 8));
    SD_SendByteSPI((uint8_t)tcacs);
}
// END  SD_SendCommand()



// Gets the R1 response returned by an SD card for a given command.
uint8_t SD_GetR1(void)
{
    uint8_t r1;
    uint8_t timeout = 0;

    while((r1 = SD_ReceiveByteSPI()) == 0xFF)
    {
        if(timeout++ >= 0xFF) return R1_TIMEOUT;
    }
    return r1;
}
// END SD_GetR1()



// Prints the results of the r1 response in readable form.
void SD_PrintR1(uint8_t r1)
{
    if(r1&R1_TIMEOUT)
        print_str(" R1_TIMEOUT,"); //Not part SD r1 response.
    if(r1&PARAMETER_ERROR)
        print_str(" PARAMETER ERROR,");
    if(r1&ADDRESS_ERROR)
        print_str(" ADDRESS ERROR,");
    if(r1&ERASE_SEQUENCE_ERROR)
        print_str(" ERASE SEQUENCE ERROR");
    if(r1&COM_CRC_ERROR)
        print_str(" COM_CRC_ERROR");
    if(r1&ILLEGAL_COMMAND)
        print_str(" ILLEGAL COMMAND");
    if(r1&ERASE_RESET)
        print_str(" ERASE RESET");
    if(r1&IN_IDLE_STATE)
        print_str(" IN IDLE STATE");
    if(r1==0) // r1 = 0 NO ERRORS;
        print_str(" OUT OF IDLE");
}
// END SD_Printr1()



// Prints the response returned by SD_InitializeSPImode() in a
void SD_PrintInitError(uint32_t err)
{
    if(err&FAILED_GO_IDLE_STATE)
        print_str(" FAILED_GO_IDLE_STATE");
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
// END sd_printInitErrors()



/******************************************************************************
 *                        "PRIVATE" FUNCTION DEFINITIONS
******************************************************************************/



/******************************************************************************
 * Description: Generates and returns CRC7 bits for SD command/argument 
 *              combination. Should only be called from SD_SendCommand()
 * Argument(s): 40-bit Transmission, Command, Argument (tca) bits to be sent as
 *              command to SD Card. 24-bit leading zeros in the argument are
 *              not used
 * Returns:     1 byte with the 7 most significant bits corresponding to the 
 *              calculated CRC7.  
 * Notes:       The value of the LSB returned does not matter, it will be 
 *              set to 1 as the transmission stop bit regardless of the value
 *              returned here.
******************************************************************************/
uint8_t pvt_CRC7(uint64_t tca)
{
    uint64_t test = 0x800000000000; // test will determine if division will
                                    // take place during a given iteration.
    
    //divisor 0b10001001 (0x89) used by SD standard to generate CRC7.
    //byte 6 of divisor variable is initialized with this value.
    uint64_t divisor = 0x890000000000;

    //initialize result with transmit/cmd/arg portion of SD command.
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
// END _CRC7()