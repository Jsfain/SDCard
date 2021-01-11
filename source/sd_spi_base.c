/*
  Implementation of SD_SPI_BASE.H

  File    : SD_SPI_BASE.C
  Version : 0.0.0.1 
  Author  : Joshua Fain
  Target  : ATMega1280
  License : MIT
  Copyright (c) 2020
*/


#include <stdint.h>
#include <avr/io.h>
#include "prints.h"
#include "spi.h"
#include "sd_spi_base.h"





/*
*******************************************************************************
*******************************************************************************
 *                     
 *                       "PRIVATE" FUNCTIONS DECLARATION
 *  
*******************************************************************************
*******************************************************************************
*/

uint8_t 
pvt_crc7 (uint64_t tca);





/*
*******************************************************************************
*******************************************************************************
 *                     
 *                                   FUNCTIONS   
 *  
*******************************************************************************
*******************************************************************************
*/

/*-----------------------------------------------------------------------------
|                                           INITIALIZE AN SD CARD INTO SPI MODE
| 
| ARG: *ctv - ptr to an instance of CTV 
| RET: Initialization Error Response.
-----------------------------------------------------------------------------*/
uint32_t 
sd_spiModeInit (CTV * ctv)
{
    uint8_t r1 = 0; //R1 response returned for every SD Command
    uint8_t r7[5];  //R7 response returned by SEND_IF_COND (CMD8)

    //Wait up to 80 clock cycles for power up to complete.
    for (uint8_t i = 0; i <= 10; i++)
      sd_sendByteSPI (0xFF);


    // ********************
    // GO_IDLE_STATE (CMD0)
    CS_SD_LOW;
    sd_sendCommand (GO_IDLE_STATE, 0);
    r1 = sd_getR1();
    CS_SD_HIGH;
    if (r1 != 1) 
      return (FAILED_GO_IDLE_STATE | r1);
    // END GO_IDLE_STATE (CMD0)
    // ************************


    // *******************
    // SEND_IF_COND (CMD8)
    uint8_t chkPtrn = 0xAA;
    uint8_t voltSuppRng  = 0x01; // 2.7 to 3.6V
    
    CS_SD_LOW;
    sd_sendCommand (SEND_IF_COND, ((uint16_t)voltSuppRng << 8) | chkPtrn);

    //Get R7 response
    r7[0] = sd_getR1(); // First R7 byte is R1 response.
    r7[1] = sd_receiveByteSPI();
    r7[2] = sd_receiveByteSPI();
    r7[3] = sd_receiveByteSPI();
    r7[4] = sd_receiveByteSPI();

    CS_SD_HIGH;
    if (r7[0] == (ILLEGAL_COMMAND | IN_IDLE_STATE)) 
      ctv->version = 1;
    else if (r7[0] == IN_IDLE_STATE) 
      {
        ctv->version = 2;
        if ((r7[3] != voltSuppRng) || (r7[4] != chkPtrn))
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
    sd_sendCommand (CRC_ON_OFF, 0);  //0 CRC OFF (default)
                                     //1 CRC ON
    r1 = sd_getR1();
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
        sd_sendCommand (APP_CMD, 0); // send APP_CMD before any ACMD
        r1 = sd_getR1();
        CS_SD_HIGH;
        if (r1 != 1) 
          return (FAILED_APP_CMD | r1);
        r1 = 0;
        CS_SD_LOW;
        sd_sendCommand (SD_SEND_OP_COND, acmd41Arg);
        r1 = sd_getR1();
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
    sd_sendCommand (READ_OCR,0);
    r1 = sd_getR1();
    if (r1 != 0)
      return (FAILED_READ_OCR | r1);

    uint8_t ocr = sd_receiveByteSPI(); 

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
    uint16_t voltagRangesAccepted = sd_receiveByteSPI();
             voltagRangesAccepted <<= 1;
             voltagRangesAccepted |= (sd_receiveByteSPI()>>7);
    

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



/*-----------------------------------------------------------------------------
|                                                      SEND BYTE TO THE SD CARD
|
| ARG : byte - byte packet to be sent to the SD card via the AVR's SPI port.
| NOTE: requires SPI.H
-----------------------------------------------------------------------------*/
void
sd_sendByteSPI (uint8_t byte)
{
  spi_masterTransmit (byte);
}



/*-----------------------------------------------------------------------------
|                                                 RECEIVE BYTE FROM THE SD CARD
|
| ARG : Void 
| RET : single byte packet received from the SD card into the AVR's SPI port.
| NOTE: Requires SPI.H
-----------------------------------------------------------------------------*/
uint8_t 
sd_receiveByteSPI (void)
{
  sd_sendByteSPI(0xFF);
  return spi_masterRead();
}



/*-----------------------------------------------------------------------------
|                                                       SEND COMMAND TO SD CARD
|
| ARG : cmd - 8-bit SD Card command to send to the SD Card.
|     : arg - 32-bit argument to be sent with the command.
-----------------------------------------------------------------------------*/
void 
sd_sendCommand (uint8_t cmd, uint32_t arg)
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
    sd_sendByteSPI (0xFF);

  // Send command to SD Card via SPI
  sd_sendByteSPI ((uint8_t)(tcacs >> 40));
  sd_sendByteSPI ((uint8_t)(tcacs >> 32));
  sd_sendByteSPI ((uint8_t)(tcacs >> 24));
  sd_sendByteSPI ((uint8_t)(tcacs >> 16));
  sd_sendByteSPI ((uint8_t)(tcacs >> 8));
  sd_sendByteSPI ((uint8_t)tcacs);
}



/*-----------------------------------------------------------------------------
|                                                            GET THE R1 RESPONSE
|
| ARG : void
| RET : R1 Response Flags
| NOTE: Call immediately after sd_sendCommand().
-----------------------------------------------------------------------------*/
uint8_t 
sd_getR1 (void)
{
  uint8_t r1;
  uint8_t timeout = 0;

  while ((r1 = sd_receiveByteSPI()) == 0xFF)
    {
      if(timeout++ >= 0xFF) 
      return R1_TIMEOUT;
    }
  return r1;
}



/*-----------------------------------------------------------------------------
|                                                         PRINT THE R1 RESPONSE
|
| ARG : r1 - R1 response returned by sd_getR1().
-----------------------------------------------------------------------------*/
void 
sd_printR1 (uint8_t r1)
{
  if (r1 & R1_TIMEOUT)
    print_str (" R1_TIMEOUT,");
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



/*-----------------------------------------------------------------------------
|                                        PRINT THE INITIALIZATION RESPONSE FLAG
|
| ARG : initResp - response returned by sd_spiModeInit().
| NOTE: only prints the initialization error flag portion of the initResp. To 
|       read the R1 portion, pass initResp to sd_printR1().
-----------------------------------------------------------------------------*/
void 
sd_printInitError (uint32_t initResp)
{
  if (initResp & FAILED_GO_IDLE_STATE)
    print_str (" FAILED_GO_IDLE_STATE,");
  if (initResp & FAILED_SEND_IF_COND)
    print_str (" FAILED_SEND_IF_COND,");
  if (initResp & UNSUPPORTED_CARD_TYPE)
    print_str (" UNSUPPORTED_CARD_TYPE,");
  if (initResp & FAILED_CRC_ON_OFF)
    print_str (" FAILED_CRC_ON_OFF,");
  if (initResp & FAILED_APP_CMD)
    print_str (" FAILED_APP_CMD,");
  if (initResp & FAILED_SD_SEND_OP_COND)
    print_str (" FAILED_SD_SEND_OP_COND,");
  if (initResp & OUT_OF_IDLE_TIMEOUT)
    print_str (" OUT_OF_IDLE_TIMEOUT,");
  if (initResp & FAILED_READ_OCR)
    print_str (" FAILED_READ_OCR,");
  if (initResp & POWER_UP_NOT_COMPLETE)
    print_str (" POWER_UP_NOT_COMPLETE,");
  if (initResp == 0)
    print_str (" INIT_SUCCESS\n\r");
}





/*
*******************************************************************************
*******************************************************************************
 *                     
 *                       "PRIVATE" FUNCTIONS DEFINITIONS
 *  
*******************************************************************************
*******************************************************************************
*/

/*
-------------------------------------------------------------------------------
|                     (PRIVAT) CALCULATE AND RETURN CRC7
|                                        
| Description : Calculates and returns the CRC7 value of a cmd/arg combo to be  
|               sent to the SD Card with the cmd/arg by sd_sendCommandSPI().
|
| Argument    : tca   - leading 40-bit TX, CMD, ARG to be sent to the SD Card.
|                       Lowest byte will be occupied by the calculated CRC7 
|                       and stop bit.
|
| Returns     : Byte. The 7 MSBs represent the CRC7 value. The 0-bit value does
|               not matter. It will be set to 1 as the stop bit when the 
|               command is transmitted.
-------------------------------------------------------------------------------
*/
uint8_t 
pvt_crc7 (uint64_t tca)
{
  // used to test if division will take
  // place during a given iteration.
  uint64_t test = 0x800000000000; 
  
  // 0b10001001 (0x89) used by SD std to generate 
  // CRC7. Byte 6 is initialized with this value.
  uint64_t divisor = 0x890000000000;

  // initialize result with tx/cmd/arg portion of SD command.
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
