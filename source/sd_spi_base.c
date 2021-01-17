/******************************************************************************
 * File    : SD_SPI_BASE.C
 * Version : 0.0.0.1 
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : MIT
 * Copyright (c) 2020-2021
 * 
 * Description:
 * Implementation of SD_SPI_BASE.H
 *****************************************************************************/


#include <stdint.h>
#include <avr/io.h>
#include "prints.h"
#include "spi.h"
#include "sd_spi_base.h"





/******************************************************************************
 ******************************************************************************
 *                     
 *                       "PRIVATE" FUNCTIONS DECLARATION
 *  
 ******************************************************************************
 *****************************************************************************/

/* For calculating the CRC7 portion of a command */
uint8_t 
pvt_crc7 (uint64_t tca);





/******************************************************************************
 ******************************************************************************
 *                     
 *                                   FUNCTIONS   
 *  
 ******************************************************************************
 *****************************************************************************/


/*-----------------------------------------------------------------------------
 *                                                           INITIALIZE SD CARD
 * 
 * DESCRIPTION: 
 * Implements the SD Card SPI mode initialization routine and sets the members 
 * of the CTV instance. 
 * 
 * ARGUMENTS: 
 * CTV *ctv  - ptr to a CTV instance whose members will be set here.
 * 
|* RETURN: 
 * uint32_t - Initialization Error Response.
 * 
 * NOTES: 
 * The returned value contains the Initialization Error Flag(s) in bits 8 to 19
 * and the most recent R1 Response in the lowest byte.
 * ------------------------------------------------------------------------- */
uint32_t 
sd_spiModeInit (CTV * ctv)
{
  uint8_t  r1 = 0;                          // R1 response
  uint8_t  r7[5];                           // R7 response: SEND_IF_COND (CMD8)
  uint8_t  timeout = 0;
  uint32_t acmd41Arg = 0;                   // arg for ACMD41.

  // OCR Variables
  uint8_t  ocr = 0;
  uint8_t  ccs = 0;
  uint8_t  uhsii = 0;
  uint8_t  co2t = 0;
  uint8_t  s18a = 0;
  uint16_t vRngAcptd = 0;


  // Wait at least 80 clock cycles for power up to complete.
  for (uint8_t i = 0; i <= 10; i++)
    sd_sendByteSPI (0xFF);


  // Step 1: GO_IDLE_STATE (CMD0)
  CS_SD_LOW;
  sd_sendCommand (GO_IDLE_STATE, 0);
  r1 = sd_getR1();
  CS_SD_HIGH;
  if (r1 != 1) 
    return (FAILED_GO_IDLE_STATE | r1);

  
  // Step 2: SEND_IF_COND (CMD8)
  uint8_t chkPtrn = 0xAA;      
  uint8_t voltSuppRng  = 0x01;              // 2.7 to 3.6V
  
  CS_SD_LOW;
  sd_sendCommand (SEND_IF_COND, (uint16_t)voltSuppRng << 8 | chkPtrn);

  // Get R7 response
  r7[0] = sd_getR1();                       // First R7 byte is R1 response.
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
    if (r7[3] != voltSuppRng || r7[4] != chkPtrn)
      return (FAILED_SEND_IF_COND | UNSUPPORTED_CARD_TYPE | r7[0]);
  }
  else  
    return (FAILED_SEND_IF_COND | r7[0]);


  // Step 3: CRC_ON_OFF (CMD59)
  r1 = 0;
  CS_SD_LOW;
  sd_sendCommand (CRC_ON_OFF, 0);           // 0 = OFF (default), 1 = ON
  r1 = sd_getR1();
  CS_SD_HIGH;
  if (r1 != 1) 
    return (FAILED_CRC_ON_OFF | r1);


  // Step 4: SD_SEND_OP_COND (ACMD41)
  // ACMD41 arg depends on card type host supports
  if (HOST_CAPACITY_SUPPORT == SDHC)
    acmd41Arg = 0x40000000;

  // Continue sending host capacity supported until card
  // signals it is no longer in the idle state or times out.
  do
  {
    r1 = 0;
    CS_SD_LOW;
    // must send APP_CMD before an ACMD.
    sd_sendCommand (APP_CMD, 0);          
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
    if (timeout++ >= 0xFE && r1 > 0)
      return (FAILED_SD_SEND_OP_COND | OUT_OF_IDLE_TIMEOUT | r1);
  }
  while (r1 & IN_IDLE_STATE);



  // Step 5: READ_OCR (CMD 58)
  // Final init step is to read CCS bit of the OCR.
  r1 = 0;
  CS_SD_LOW;
  sd_sendCommand (READ_OCR, 0);
  r1 = sd_getR1();
  if (r1 != 0)
    return (FAILED_READ_OCR | r1);
  ocr = sd_receiveByteSPI(); 

  // power up status
  if (ocr >> 7 != 1)
  {
    CS_SD_HIGH;
    return (POWER_UP_NOT_COMPLETE | r1);
  }
  
  // Only CCS is needed to complete initialization, but
  // verifying rest of OCR returned is valid for host.
  ccs   = (ocr & 0x40) >> 6;
  uhsii = (ocr & 0x20) >> 5;
  co2t  = (ocr & 0x10) >> 3;
  s18a  = (ocr & 0x01);
  vRngAcptd = sd_receiveByteSPI();
  vRngAcptd <<= 1;
  vRngAcptd |= sd_receiveByteSPI() >> 7;
  
  if (ccs == 1)
    ctv->type = SDHC;
  else 
    ctv->type = SDSC;

  if (uhsii != 0 ||                         // UHSII not supported
      co2t  != 0 ||                         // Over 2TB not supported
      s18a  != 0 ||                         // Switch to 1.8V not supported
      vRngAcptd != 0x1FF)                   // 2.7 to 3.6V supported.
  {
    CS_SD_HIGH;
    return (FAILED_READ_OCR | UNSUPPORTED_CARD_TYPE | r1);
  }
  CS_SD_HIGH;
  
  return 0;                                 // Initialization succeded
}



/*-----------------------------------------------------------------------------
 *                                                                    SEND BYTE
 * 
 * DESCRIPTION: 
 * Sends a single byte to the SD card via the SPI port.
 * 
 * ARGUMENT:
 * uint8_t byte  - 8-bit packet that will be sent to the SD Card via SPI.
 * 
 * RETURN:
 * void
 * 
 * NOTES: 
 * 1) Call this function as many times as required to send the complete data 
 *    packet, token, command, etc...
 * 2) This, along with sd_receiveByteSPI(), are the SPI interfacing functions.
 * ------------------------------------------------------------------------- */
void
sd_sendByteSPI (uint8_t byte)
{
  spi_masterTransmit (byte);
}



/*-----------------------------------------------------------------------------
 *                                                                 RECEIVE BYTE
 * 
 * DESCRIPTION: 
 * Receives and returns a single byte from the SD card via the SPI port.
 * 
 * ARGUMENT:
 * void
 * 
 * RETURN:
 * uint8_t - Single byte received from the SD card.
 * 
 * NOTE:
 * 1) Call this function as many times as necessary to get the complete data
 *    packet, token, error response, etc... received by the SD card.
 * 2) This, along with sd_sendByteSPI(), are the SPI interfacing functions.
 * ------------------------------------------------------------------------- */
uint8_t 
sd_receiveByteSPI (void)
{
  sd_sendByteSPI (0xFF);
  return spi_masterRead();
}



/*-----------------------------------------------------------------------------
 *                                                                 SEND COMMAND
 * 
 * DESCRIPTION: 
 * Send a command and argument to the SD Card.
 * 
 * ARGUMENT:
 * 1) uint8_t  cmd - SD Card command. See sd_spi_cmds.h
 * 2) uint32_t arg - 32-bit argument to be sent with the command. 
 * 
 * RETURN:
 * void
 * ------------------------------------------------------------------------- */
void 
sd_sendCommand (uint8_t cmd, uint32_t arg)
{
  // Prepare the command to be sent. Total of 48 bits. 
  // The structure of a command (MSB --> LSB) is:
  // TRANSMIT (2b) = 0b01 | CMD (6b) | ARG (32b) | CRC7 (7b) | STOP (1b) = 0b1
  uint8_t  tc = 0x40 | cmd;
  uint64_t tcacs = 0;
  uint8_t  crc = 0;                   

  tcacs = (tcacs | tc) << 40;
  tcacs =  tcacs | (uint64_t)arg << 8;
  crc = pvt_crc7 (tcacs);
  tcacs = (tcacs | crc | 1);                // load CRC and STOP bit.

  // Found that sometimes introducing delay
  // in sending command improves stability.
  for (uint8_t i = 0; i < 10; i++)
    sd_sendByteSPI (0xFF);

  // Send command to SD Card via SPI port
  sd_sendByteSPI ((uint8_t)(tcacs >> 40));
  sd_sendByteSPI ((uint8_t)(tcacs >> 32));
  sd_sendByteSPI ((uint8_t)(tcacs >> 24));
  sd_sendByteSPI ((uint8_t)(tcacs >> 16));
  sd_sendByteSPI ((uint8_t)(tcacs >> 8));
  sd_sendByteSPI ((uint8_t)(tcacs));
}



/*-----------------------------------------------------------------------------
 *                                                              GET R1 RESPONSE
 * 
 * DESCRIPTION: 
 * Gets the R1 response from the SD card after sending it a SD command.
 * 
 * ARGUMENT:
 * void 
 * 
 * RETURN:
 * uint8_t - R1 response flags.
 * 
 * NOTES:
 * 1) Always call this function immediately after sd_sendCommand().
 * 1) Pass the return value to sd_printR1(err) to print the R1 response.
 * 2) If R1_TIMEOUT is returned, then SD Card did not return an R1 response.
 * ------------------------------------------------------------------------- */
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
 *                                                      PRINT R1 RESPONSE FLAGS
 * 
 * DESCRIPTION: 
 * Prints the R1 response flag(s) returned by sd_getR1().
 * 
 * ARGUMENT:
 * uint8_t r1 - R1 response flags byte returned by sd_getR1(). 
 * 
 * RETURN:
 * void
 * ------------------------------------------------------------------------- */
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
 *                                          PRINT INITIALIZATION RESPONSE FLAGS
 * 
 * DESCRIPTION: 
 * Prints Initialization Error Flag portion of the sd_spiModeInit() response.
 *  
 * ARGUMENT:
 * uint32_t initErr - The Initialization Error Response returned by the 
 *                    initialization routine, sd_spiModeInit().
 * 
 * RETURN:
 * void
 * 
 * NOTES:
 * Though this will only print bits 8 to 19 of the sd_spiModeInit() function's
 * returned value (the Initialization Error Response), the entire returned
 * value can be passed to this function without issue. Bits 0 to 7 correspond
 * to the R1 Response portion of the Initialization Response. To read this
 * portion pass the Initialization Error Response to sd_printR1(initErr).
 * ------------------------------------------------------------------------- */
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
 ******************************************************************************
 ******************************************************************************
 *                     
 *                       "PRIVATE" FUNCTIONS DEFINITIONS
 *  
 ******************************************************************************
 ******************************************************************************
 */


/*-----------------------------------------------------------------------------
 *                                                    CALCULATE AND RETURN CRC7
 * 
 * DESCRIPTION: 
 * Used by sd_sendCommand() to calculate the CRC7 for a command.
 * 
 * ARGUMENT:
 * uint64_t tca - The tx/cmd/arg bits used to calculate the CRC7.
 * 
 * RETURN:
 * uint8_t - calculated CRC7 bits to be sent with the SD command.
 * ------------------------------------------------------------------------- */
uint8_t 
pvt_crc7 (uint64_t tca)
{
  // used to test if division will take place during a given iteration.
  uint64_t test = 0x800000000000; 
  
  // 0b10001001 (0x89) used by SD std to generate CRC7.
  // Byte 6 is initialized with this value.
  uint64_t divisor = 0x890000000000;

  // initialize result with tx/cmd/arg portion of SD command.
  uint64_t result = tca;

  // calculate CRC7
  for (int i = 0; i < 40; i++)
  {
    if (result&test)
      result ^= divisor;
    divisor >>= 1;
    test >>= 1;
  }
  return result;
}
