/*
***********************************************************************************************************************
*                                                   AVR-SD CARD MODULE
*
* File    : SD_SPI_BASE.C
* Version : 0.0.0.1 
* Author  : Joshua Fain
* Target  : ATMega1280
*
*
* DESCRIPTION:
* Defines base-level SPI mode SD card functions to handle the basic physical interaction of the AVR microcontroller
* with an SD card operating in SPI Mode.
*                                                 
*
* FUNCTIONS "PUBLIC":
*  (1) uint32_t sd_spi_mode_init (CTV * ctv)
*  (2) void     sd_spi_send_byte (uint8_t byte)
*  (3) uint8_t  sd_spi_receive_byte (void)
*  (4) uvoid    sd_spi_send_command (uint8_t cmd, uint32_t arg)
*  (5) uint8_t  sd_spi_get_r1 (void)
*  (6) void     sd_spi_print_r1 (uint8_t r1)
*  (7) void     sd_spi_print_init_error (uint32_t err)
*
*
* STRUCTS (defined in SD_SPI_BASE.H)
*   typedef struct CardTypeVersion CTV;
*
* 
*                                                      MIT LICENSE
*
* Copyright (c) 2020 Joshua Fain
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
* documentation files (the "Software"), to deal in the Software without restriction, including without limitation the 
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to 
* permit ersons to whom the Software is furnished to do so, subject to the following conditions: The above copyright 
* notice and this permission notice shall be included in all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***********************************************************************************************************************
*/


#include <stdint.h>
#include <avr/io.h>
#include "../includes/sd_spi_base.h"
#include "../includes/spi.h"
#include "../includes/prints.h"



/*
***********************************************************************************************************************
 *                                            "PRIVATE" FUNCTION DECLARATION
***********************************************************************************************************************
*/

// Private functions are described at the bottom of this file with their definitions.

uint8_t 
pvt_crc7 (uint64_t tca);



/*
***********************************************************************************************************************
 *                                            "PUBLIC" FUNCTION DEFINITIONS
***********************************************************************************************************************
*/

/*
***********************************************************************************************************************
 *                                         INITIALIZE AN SD CARD INTO SPI MODE
 * 
 * Description : This function must be called first before implementing any other part of the AVR-SD Card module. This
 *               function will initialize the SD Card into SPI mode and set the CTV struct instance members to the 
 *               correct card type and version. 
 * 
 * Argument    : *ctv      Pointer to an instance of a CTV struct. This function will set the members of this instance.
 *                         The setting of 'type' is critical for block address handling.
 * 
 * Return      : Initialization Error Response      The initialization response will include an Initialization Error 
 *                                                  Flag and the most recent R1 Response Flag. The Initialization Error
 *                                                  flag will be set in bits 8 to 19 of the returned value. The R1 
 *                                                  Response flags will be set in bits 0 to 7. These can be read by 
 *                                                  passing the returned value to sd_spi_print_init_error(err) and
 *                                                  sd_spi_print_error (r1), respectively.
***********************************************************************************************************************
*/

uint32_t 
sd_spi_mode_init (CTV * ctv)
{
    uint8_t r1 = 0; //R1 response returned for every SD Command
    uint8_t r7[5];  //R7 response returned by SEND_IF_COND (CMD8)

    //Wait up to 80 clock cycles for power up to complete.
    for (uint8_t i = 0; i <= 10; i++)
      sd_spi_send_byte (0xFF);


    // ********************
    // GO_IDLE_STATE (CMD0)
    CS_SD_LOW;
    sd_spi_send_command (GO_IDLE_STATE, 0);
    r1 = sd_spi_get_r1();
    CS_SD_HIGH;
    if (r1 != 1) 
      return (FAILED_GO_IDLE_STATE | r1);
    // END GO_IDLE_STATE (CMD0)
    // ************************

      print_str("\n\r HERE 3");
    // *******************
    // SEND_IF_COND (CMD8)
    uint8_t checkPattern = 0xAA;
    uint8_t voltageSupplyRange  = 0x01; // 2.7 to 3.6V
    
    CS_SD_LOW;
    sd_spi_send_command (SEND_IF_COND, ((uint16_t)voltageSupplyRange << 8) | checkPattern);

    //Get R7 response
    r7[0] = sd_spi_get_r1(); // First R7 byte is R1 response.
    r7[1] = sd_spi_receive_byte();
    r7[2] = sd_spi_receive_byte();
    r7[3] = sd_spi_receive_byte();
    r7[4] = sd_spi_receive_byte();

    CS_SD_HIGH;
    if (r7[0] == (ILLEGAL_COMMAND | IN_IDLE_STATE)) 
      ctv->version = 1;
    else if (r7[0] == IN_IDLE_STATE) 
      {
        ctv->version = 2;
        if ((r7[3] != voltageSupplyRange) || (r7[4] != checkPattern))
          return (FAILED_SEND_IF_COND | UNSUPPORTED_CARD_TYPE | r7[0]);
      }
    else  
      return (FAILED_SEND_IF_COND | r7[0]);
    // END SEND_IF_COND (CMD8)
    // ***********************


    // ******************
    // CRC_ON_OFF (CMD59)
    r1 = 0;
    CS_SD_LOW;
    sd_spi_send_command (CRC_ON_OFF, 0);  //0 CRC OFF (default)
                                     //1 CRC ON
    r1 = sd_spi_get_r1();
    CS_SD_HIGH;
    if (r1 != 1) 
      return (FAILED_CRC_ON_OFF | r1);
    // END CRC_ON_OFF (CMD59)
    // **********************


    // ************************
    // SD_SEND_OP_COND (ACMD41)
    uint32_t acmd41Arg;

    // HCS - High Capacity Supported by host. Default TRUE.
    if (HCS) 
      acmd41Arg = 0x40000000; 
    else 
      acmd41Arg = 0; 

    // Continue sending host capacity info to SD card until card
    // signals it is no longer in the idle state or times out.
    uint8_t timeout = 0;
    do
      {
        r1 = 0;
        CS_SD_LOW;
        sd_spi_send_command (APP_CMD, 0); // send APP_CMD before any ACMD
        r1 = sd_spi_get_r1();
        CS_SD_HIGH;
        if (r1 != 1) 
          return (FAILED_APP_CMD | r1);
        r1 = 0;
        CS_SD_LOW;
        sd_spi_send_command (SD_SEND_OP_COND, acmd41Arg);
        r1 = sd_spi_get_r1();
        CS_SD_HIGH;
        if (r1 > 1)
          return (FAILED_SD_SEND_OP_COND | r1);
        if ((timeout++ >= 0xFE) && (r1 > 0))
          return (FAILED_SD_SEND_OP_COND | OUT_OF_IDLE_TIMEOUT | r1);
      }
    while (r1 & IN_IDLE_STATE);
    // END SEND_OP_COND (ACMD41)
    // *************************


    // *****************
    // READ_OCR (CMD 58)

    // Final init step is to read CCS (Card Capactity Status)
    // bit of the OCR (Operating Condition Register).
    r1 = 0;
    CS_SD_LOW;
    sd_spi_send_command (READ_OCR,0);
    r1 = sd_spi_get_r1();
    if (r1 != 0)
      return (FAILED_READ_OCR | r1);

    uint8_t ocr = sd_spi_receive_byte(); 

    //POWER_UP_STATUS
    if( (ocr >> 7) != 1 )
      {
        CS_SD_HIGH;
        return (POWER_UP_NOT_COMPLETE | r1);
      }
    
    // Confirm rest of OCR returned is valid, though, technically
    // only CCS is needed to complete initialization.
    uint8_t  ccs   = (ocr&0x40) >> 6;
    uint8_t  uhsii = (ocr&0x20) >> 5;
    uint8_t  co2t  = (ocr&0x10) >> 3;
    uint8_t  s18a  = (ocr&0x01);
    uint16_t voltagRangesAccepted = sd_spi_receive_byte();
             voltagRangesAccepted <<= 1;
             voltagRangesAccepted |= (sd_spi_receive_byte()>>7);
    

    if (ccs == 1) 
      ctv->type = SDHC;
    else 
      ctv->type = SDSC;

    if ((uhsii !=  0) || // UHSII not supported
        (co2t  !=  0) || // Over 2TB not supported
        (s18a  !=  0) || // Switching to 1.8V not supported
        (voltagRangesAccepted != 0x1FF)  ) // Voltage range of
                                           // 2.7 to 3.6V supported.
      {
        CS_SD_HIGH;
        return (FAILED_READ_OCR | UNSUPPORTED_CARD_TYPE | r1);
      }
    CS_SD_HIGH;
    // END READ_OCR (CMD58)
    // ************************
    
    return 0; //initialization succeded
}
// END SD_InitializeSPImode(...)



/*
***********************************************************************************************************************
 *                                               SEND BYTE TO THE SD CARD
 * 
 * Description : This function sends a single byte to the SD Card via the SPI port. This function along with
 *               sd_spi_receive_byte() are the SPI port interfacing functions. Any interaction with the SD card 
 *               directly uses these functions. 
 * 
 * Argument    : byte      The 8 bit packet that will be sent to the SD Card via the SPI port.
 * 
 * Note        : This function should be called as many times as required in order to send a complete data packet, 
 *               token, command, etc...
***********************************************************************************************************************
*/

void 
sd_spi_send_byte (uint8_t byte)
{
  spi_masterTransmit (byte);
}
// END sd_spi_send_byte



/*
***********************************************************************************************************************
 *                                           RECEIVE BYTE FROM THE SD CARD
 * 
 * Description : This function is used to get a single byte sent by the SD Card sent by via the SPI port. This function
 *               along with sd_spi_send_byte() are the SPI port interfacing functions. Any interaction with the SD card 
 *               directly uses these functions. 
 *
 * Argument    : void
 * 
 * Return      : 8 bit packet that will be sent by the SD card via the SPI port.
 * 
 * Note        : This function should be called as many times as required in order to get the complete data packet, 
 *               token, error response, etc..., sent by the SD Card.
***********************************************************************************************************************
*/

uint8_t 
sd_spi_receive_byte (void)
{
  sd_spi_send_byte(0xFF);
  return spi_masterRead();
}
// END sd_spi_receive_byte



/*
***********************************************************************************************************************
 *                                               SEND COMMAND TO SD CARD
 * 
 * Description : Send a command and argument to the SD Card.
 * 
 * Argument    : cmd      - 8 bit unsigned integer specifying the command that the function will send to the SD Card. 
 *             : arg      - Argument to be sent with the command to the SD Card.
 **********************************************************************************************************************
*/

void 
sd_spi_send_command (uint8_t cmd, uint32_t arg)
{
  // Prepare the command / argument to send. Total of 48 bits. 
  // The structure of command / argument (MSB --> LSB) is:
  // TRANSMIT (2b) = 0b01 | CMD (6b) | ARG (32b) | CRC7 (7b) | STOP (1b) = 0b1
  uint8_t  tc = 0x40 | cmd;
  uint64_t tcacs = 0;                      

  tcacs = (tcacs | tc) << 40;
  tcacs =  tcacs | ((uint64_t)(arg) << 8);
  uint8_t crc = pvt_crc7(tcacs);
  tcacs = (tcacs | crc | 1);  //load CRC and STOP bit.

  // delay sending command.
  for (uint8_t i = 0; i <= 10; i++)
    sd_spi_send_byte (0xFF);

  // Send command to SD Card via SPI
  sd_spi_send_byte ((uint8_t)(tcacs >> 40));
  sd_spi_send_byte ((uint8_t)(tcacs >> 32));
  sd_spi_send_byte ((uint8_t)(tcacs >> 24));
  sd_spi_send_byte ((uint8_t)(tcacs >> 16));
  sd_spi_send_byte ((uint8_t)(tcacs >> 8));
  sd_spi_send_byte ((uint8_t)tcacs);
}
// END  sd_spi_send_command



/*
***********************************************************************************************************************
 *                                                 GET THE R1 RESPONSE
 * 
 * Description : Always call this function immediately after sd_spi_send_command() to get the retured R1 response.
 * 
 * Argument    : void
 * 
 * Return      : R1 Response Flags
 * 
 * Note        : Call sd_spi_print_r1(r1) to read the R1 response. 
***********************************************************************************************************************
*/

uint8_t 
sd_spi_get_r1 (void)
{
  uint8_t r1;
  uint8_t timeout = 0;

  while ((r1 = sd_spi_receive_byte()) == 0xFF)
    {
      if(timeout++ >= 0xFF) 
      return R1_TIMEOUT;
    }
  return r1;
}
// END sd_spi_get_r1



/*
***********************************************************************************************************************
 *                                                 PRINT THE R1 RESPONSE
 * 
 * Description : Call this function to print the R1 response returned by sd_spi_get_r1().
 * 
 * Argument    : r1           R1 response returned by sd_spi_get_r1();
***********************************************************************************************************************
*/

void 
sd_spi_print_r1 (uint8_t r1)
{
  if (r1 & R1_TIMEOUT)
    print_str (" R1_TIMEOUT,"); // Not actual flag returned by SD Card.
  if (r1 & PARAMETER_ERROR)
    print_str (" PARAMETER_ERROR,");
  if (r1 & ADDRESS_ERROR)
    print_str (" ADDRESS_ERROR,");
  if (r1 & ERASE_SEQUENCE_ERROR)
    print_str (" ERASE_SEQUENCE_ERROR,");
  if (r1 & COM_CRC_ERROR)
    print_str (" COM_CRC_ERROR,");
  if (r1 & ILLEGAL_COMMAND)
    print_str (" ILLEGAL_COMMAND,");
  if (r1 & ERASE_RESET)
    print_str (" ERASE_RESET,");
  if (r1 & IN_IDLE_STATE)
    print_str (" IN_IDLE_STATE");
  if (r1 == 0)
    print_str (" OUT_OF_IDLE");
}
// END SD_Printr1



/*
***********************************************************************************************************************
 *                                        PRINT THE INITIALIZATION RESPONSE FLAG
 * 
 * Description : Call this function to print the Initialization Error Response portion of the value returned by the
 *               SD Card SPI mode initialization routine. This will only check bits 8 to 19 of the intialization
 *               returned value as the lowest byte (bits 0 to 8) are the most recent R1 response returned during 
 *               initialization, and should be read by sd_spi_print_r1(). 
 * 
 * Argument    : err          Initialization error response.
 ***********************************************************************************************************************
*/

void 
sd_spi_print_init_error (uint32_t err)
{
  if (err & FAILED_GO_IDLE_STATE)
    print_str (" FAILED_GO_IDLE_STATE,");
  if (err & FAILED_SEND_IF_COND)
    print_str (" FAILED_SEND_IF_COND,");
  if (err & UNSUPPORTED_CARD_TYPE)
    print_str (" UNSUPPORTED_CARD_TYPE,");
  if (err & FAILED_CRC_ON_OFF)
    print_str (" FAILED_CRC_ON_OFF,");
  if (err & FAILED_APP_CMD)
    print_str (" FAILED_APP_CMD,");
  if (err & FAILED_SD_SEND_OP_COND)
    print_str (" FAILED_SD_SEND_OP_COND,");
  if (err & OUT_OF_IDLE_TIMEOUT)
    print_str (" OUT_OF_IDLE_TIMEOUT,");
  if (err & FAILED_READ_OCR)
    print_str (" FAILED_READ_OCR,");
  if (err & POWER_UP_NOT_COMPLETE)
    print_str (" POWER_UP_NOT_COMPLETE,");
  if (err == 0)
    print_str (" INIT_SUCCESS\n\r");
}
// END sd_printInitErrors



/*
***********************************************************************************************************************
 *                                           "PRIVATE" FUNCTION DEFINITION
***********************************************************************************************************************
*/

/*
***********************************************************************************************************************
 *                                      PRINT THE INITIALIZATION RESPONSE FLAG
 * 
 * Description : Private function used to generate and return the CRC7 bits for an SD Card command/argument combo.
 *               This function should only be called from sd_spi_send_command()
 * 
 * Argument    : tca          40-bit Transmission, Command, Argument bits to be sent to the SD Card; Total is 48-bits. 
 *                            16 leading bits of the data type are not used. The function will generate the valid
 *                            CRC7 value based on the value of the tca argument.
 * 
 * Returns     : Unsigned byte. The upper 7 bits represent the CRC7 value. The LSBit value does not matter as it will 
 *               be set to 1 as the STOP bit once the CRC7 byte is returned to the sd_spi_send_command() function.
 ***********************************************************************************************************************
*/

uint8_t 
pvt_crc7 (uint64_t tca)
{
  // 'test' will determine if division will take place during a given iteration.
  uint64_t test = 0x800000000000; 
  
  // divisor 0b10001001 (0x89) used by SD std. to generate CRC7. Byte 6 is initialized with this value.
  uint64_t divisor = 0x890000000000;

  // initialize result with transmit/cmd/arg portion of SD command.
  uint64_t result = tca;

  // calculate CRC7
  for (int i = 0; i < 40; i++)
    {
      if(result&test)
        result ^= divisor;
      divisor >>= 1;
      test >>= 1;
    }
  return result;
}
// END pvt_crc7
