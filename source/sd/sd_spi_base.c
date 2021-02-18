/*
 * File    : SD_SPI_BASE.C
 * Version : 0.0.0.1 
 * Author  : Joshua Fain
 * Target  : ATMega1280
 * License : MIT
 * Copyright (c) 2020-2021
 *  
 * Implementation of SD_SPI_BASE.H
 */

#include <stdint.h>
#include <avr/io.h>
#include "prints.h"
#include "spi.h"
#include "sd_spi_base.h"

/*
 ******************************************************************************
 *                        "PRIVATE" FUNCTION PROTOTYPES
 ******************************************************************************
 */

static uint8_t pvt_CRC7(uint64_t tca);

/*
 ******************************************************************************
 *                                   FUNCTIONS   
 ******************************************************************************
 */

/*
 * ----------------------------------------------------------------------------
 *                                                       SD CARD INITIALIZATION
 *
 * Description : Implements the SD Card SPI mode initialization routine and 
 *               sets the members of the CTV instance. 
 *
 * Arguments   : ctv     ptr to a CTV instance whose members will be set here.
 * 
 * Returns     : Initialization Error Response. This includes initialization 
 *               error flag in bits 8 to 19 and the most recent R1 response in
 *               the lowest byte.
 * ----------------------------------------------------------------------------
 */
uint32_t sd_InitModeSPI(CTV* ctv)
{
  // R1 response is first byte returned by SD card in response to every command
  uint8_t r1;                         

  // Wait at least 80 clock cycles for power up to complete.
  for (uint8_t waitCnt = 0; waitCnt <= 10; waitCnt++)
    sd_SendByteSPI(0xFF);

  //
  // Step 1: GO_IDLE_STATE (CMD0)
  //
  CS_SD_LOW;
  sd_SendCommand(GO_IDLE_STATE, 0);
  r1 = sd_GetR1();
  CS_SD_HIGH;
  if (r1 != IN_IDLE_STATE) 
    return (FAILED_GO_IDLE_STATE | r1);

  //
  // Step 2: SEND_IF_COND (CMD8)
  //
  uint8_t r7[5];                            // R7 response only for CMD8
  const uint8_t chkPtrn = 0xAA;             // check pattern is arbitrary value
  const uint8_t voltSuppRng = 0x01;         // 2.7 to 3.6V
  
  CS_SD_LOW;
  sd_SendCommand(SEND_IF_COND, (uint16_t)voltSuppRng << 8 | chkPtrn);

  // Get R7 response. First R7 byte is the R1 response.
  r7[0] = sd_GetR1();                      
  r7[1] = sd_ReceiveByteSPI();
  r7[2] = sd_ReceiveByteSPI();
  r7[3] = sd_ReceiveByteSPI();
  r7[4] = sd_ReceiveByteSPI();

  CS_SD_HIGH;
  if (r7[0] == (ILLEGAL_COMMAND | IN_IDLE_STATE)) 
    ctv->version = VERSION_1;
  else if (r7[0] == IN_IDLE_STATE) 
  {
    ctv->version = VERSION_2;
    if (r7[3] != voltSuppRng || r7[4] != chkPtrn)
      return (FAILED_SEND_IF_COND | UNSUPPORTED_CARD_TYPE | r7[0]);
  }
  else  
    return (FAILED_SEND_IF_COND | r7[0]);

  //
  // Step 3: CRC_ON_OFF (CMD59)
  //
  CS_SD_LOW;
  sd_SendCommand(CRC_ON_OFF, 0);           // 0 = OFF (default), 1 = ON
  r1 = sd_GetR1();
  CS_SD_HIGH;
  if (r1 != IN_IDLE_STATE) 
    return (FAILED_CRC_ON_OFF | r1);

  //
  // Step 4: SD_SEND_OP_COND (ACMD41)
  //
  uint8_t  timeout = 0;
  uint32_t acmd41Arg = 0;      // ACMD41 arg depends on card type host supports
  if (HOST_CAPACITY_SUPPORT == SDHC)
    acmd41Arg = 0x40000000;

  //
  // Send SD_SEND_OP_COND with acmd41arg argument to indicate to the card the
  // card capacity the host supports (SDHC or SDSC). Since SD_SEND_OP_COND is 
  // an ACMD type command, the command, APP_CMD, must first be sent to signal 
  // to the SD card that the next command is of type ACMD. This process of send
  // APP_CMD then SD_SEND_OP_COND continues to repeat until the R1 response to 
  // SD_SEND_OP_COND signals the card is no longer in the idle state.
  //
  do
  {
    CS_SD_LOW;
    sd_SendCommand(APP_CMD, 0);      // send APP_CMD before ACMD type commands.
    r1 = sd_GetR1();
    CS_SD_HIGH;
    if (r1 != IN_IDLE_STATE) 
      return (FAILED_APP_CMD | r1);
    CS_SD_LOW;
    sd_SendCommand(SD_SEND_OP_COND, acmd41Arg);      // ACMD41  
    r1 = sd_GetR1();
    CS_SD_HIGH;
    if (r1 > IN_IDLE_STATE)
      return (FAILED_SD_SEND_OP_COND | r1);
    if (timeout++ >= 0xFE && r1 > 0)
      return (FAILED_SD_SEND_OP_COND | OUT_OF_IDLE_TIMEOUT | r1);
  }
  while (r1 & IN_IDLE_STATE);


  //
  // Step 5: READ_OCR (CMD 58)
  // The final init step is to read CCS bit of the OCR.
  //

  // OCR Variables
  uint8_t  ocr, ccs, uhsii, co2t, s18a;     
  uint16_t vRngAcptd;
  
  CS_SD_LOW;
  sd_SendCommand(READ_OCR, 0);
  r1 = sd_GetR1();
  if (r1 != OUT_OF_IDLE)
    return (FAILED_READ_OCR | r1);
  ocr = sd_ReceiveByteSPI(); 

  // power up status
  if (ocr >> 7 != 1)
  {
    CS_SD_HIGH;
    return (POWER_UP_NOT_COMPLETE | r1);
  }
  
  // Only CCS is needed to complete initialization.
  ccs   = (ocr & 0x40) >> 6;
  uhsii = (ocr & 0x20) >> 5;
  co2t  = (ocr & 0x10) >> 3;
  s18a  = (ocr & 0x01);
  vRngAcptd = sd_ReceiveByteSPI();
  vRngAcptd <<= 1;
  vRngAcptd |= sd_ReceiveByteSPI() >> 7;
  
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

/*
 * ----------------------------------------------------------------------------
 *                                                                    SEND BYTE
 * 
 * Description : Sends a single byte to the SD card via the SPI port.
 * 
 * Arguments   : byte     8-bits that will be sent to the SD Card via SPI.
 * 
 * Returns     : void
 * 
 * Notes       : 1) Call as many times as required to send the complete data 
 *                  packet, token, command, etc...
 * 
 *               2) This, and sd_ReceiveByteSPI(), are the SPI interfacing 
 *                  functions.
 * ----------------------------------------------------------------------------
 */
void sd_SendByteSPI(uint8_t byte)
{
  spi_MasterTransmit(byte);
}

/*
 * ----------------------------------------------------------------------------
 *                                                                 RECEIVE BYTE
 * 
 * Description : Receives/returns single byte from the SD card via SPI port.
 * 
 * Arguments   : void
 * 
 * Returns     : 8-bit byte received from the SD card.
 * 
 * Notes       : 1) Call as many times as necessary to get the complete data
 *                  packet, token, error response, etc... from the SD card.
 * 
 *               2) This, and sd_SendByteSPI(), are the SPI interfacing 
 *                  functions.
 * ----------------------------------------------------------------------------
 */
uint8_t sd_ReceiveByteSPI(void)
{
  sd_SendByteSPI(0xFF);
  return spi_MasterReceive();
}

/*
 * ----------------------------------------------------------------------------
 *                                                                 SEND COMMAND
 * 
 * Description : Send a command and argument to the SD Card.
 * 
 * Arguments   : cmd     SD Card command. See sd_spi_cmds.h.
 * 
 *               arg     32-bit argument to be sent with the SD command.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */
void sd_SendCommand(uint8_t cmd, uint32_t arg)
{
  // Found forcing some delay between commands can improves stability.
  for (uint8_t waitCnt = 0; waitCnt < 10; waitCnt++)
    sd_SendByteSPI(0xFF);
  
  //
  // Prepare the command to be sent. Total of 48 bits. The structure of a 
  // command from MSB --> LSB is:
  // TRANSMIT (2b) = 0b01 | CMD (6b) | ARG (32b) | CRC7 (7b) | STOP (1b) = 0b1
  //
  uint64_t tcacs = 0;                       // full tx, cmd, arg, crc, stop                          

  tcacs |= (uint64_t)(0x40 | cmd) << 40;    // tx and cmd
  tcacs |= (uint64_t)arg << 8;              // arg
  tcacs |= pvt_CRC7(tcacs);                 // crc7
  tcacs |= 1;                               // stop bit

  // Send command to SD Card via SPI port
  sd_SendByteSPI ((uint8_t)(tcacs >> 40));
  sd_SendByteSPI ((uint8_t)(tcacs >> 32));
  sd_SendByteSPI ((uint8_t)(tcacs >> 24));
  sd_SendByteSPI ((uint8_t)(tcacs >> 16));
  sd_SendByteSPI ((uint8_t)(tcacs >> 8));
  sd_SendByteSPI ((uint8_t)(tcacs));
}

/*
 * ----------------------------------------------------------------------------
 *                                                              GET R1 RESPONSE
 * 
 * Description : Gets the R1 response from the SD card after it has been sent 
 *               an SD command.
 * 
 * Arguments   : void
 * 
 * Returns     : R1 response flag(s). See SD_SPI_BASE.H.
 * 
 * Notes       : 1) Always call immediately after sd_SendCommand().
 *               
 *               2) Pass the return value to sd_PrintR1() to print the value.
 * 
 *               3) R1_TIMEOUT is returned if the SD Card did not return an R1
 *                  response. 
 * ----------------------------------------------------------------------------
 */
uint8_t sd_GetR1(void)
{
  uint8_t r1;
  uint8_t timeout = 0;

  while ((r1 = sd_ReceiveByteSPI()) == 0xFF)
  {
    if(timeout++ >= 0xFF) 
    return R1_TIMEOUT;
  }
  return r1;
}

/*
 * ----------------------------------------------------------------------------
 *                                                      PRINT R1 RESPONSE FLAGS
 * 
 * Description : Prints the R1 response flag(s) returned by sd_GetR1().
 * 
 * Arguments   : r1     The R1 response flag(s) byte returned by sd_GetR1().
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */
void sd_PrintR1(uint8_t r1)
{
  if (r1 & R1_TIMEOUT)
    print_Str (" R1_TIMEOUT,");
  if (r1 & PARAMETER_ERROR)
    print_Str (" PARAMETER_ERROR,");
  if (r1 & ADDRESS_ERROR)
    print_Str (" ADDRESS_ERROR,");
  if (r1 & ERASE_SEQUENCE_ERROR)
    print_Str (" ERASE_SEQUENCE_ERROR,");
  if (r1 & COM_CRC_ERROR)
    print_Str (" COM_CRC_ERROR,");
  if (r1 & ILLEGAL_COMMAND)
    print_Str (" ILLEGAL_COMMAND,");
  if (r1 & ERASE_RESET)
    print_Str (" ERASE_RESET,");
  if (r1 & IN_IDLE_STATE)
    print_Str (" IN_IDLE_STATE");
  if (r1 == OUT_OF_IDLE) // 0
    print_Str (" OUT_OF_IDLE");
}

/*
 * ----------------------------------------------------------------------------
 *                                          PRINT INITIALIZATION RESPONSE FLAGS
 * 
 * Description : Prints Initialization Error Flag portion of the 
 *               sd_spiModeInit() response.
 * 
 * Arguments   : initResp     The Initialization Error Response returned by the
 *                            initialization routine, sd_spiModeInit().
 * 
 * Returns     : void
 * 
 * Notes       : This will only interpret bits 8 to 19 of the sd_spiModeInit() 
 *               function's returned value. The entire returned value can be 
 *               passed to this function without issue. Bits 0 to 7 correspond
 *               to the R1 Response portion of the Initialization Response. To 
 *               read the R1 portion pass initResp to sd_PrintR1().
 * ----------------------------------------------------------------------------
 */
void sd_PrintInitError(uint32_t initResp)
{
  if (initResp & FAILED_GO_IDLE_STATE)
    print_Str (" FAILED_GO_IDLE_STATE,");
  if (initResp & FAILED_SEND_IF_COND)
    print_Str (" FAILED_SEND_IF_COND,");
  if (initResp & UNSUPPORTED_CARD_TYPE)
    print_Str (" UNSUPPORTED_CARD_TYPE,");
  if (initResp & FAILED_CRC_ON_OFF)
    print_Str (" FAILED_CRC_ON_OFF,");
  if (initResp & FAILED_APP_CMD)
    print_Str (" FAILED_APP_CMD,");
  if (initResp & FAILED_SD_SEND_OP_COND)
    print_Str (" FAILED_SD_SEND_OP_COND,");
  if (initResp & OUT_OF_IDLE_TIMEOUT)
    print_Str (" OUT_OF_IDLE_TIMEOUT,");
  if (initResp & FAILED_READ_OCR)
    print_Str (" FAILED_READ_OCR,");
  if (initResp & POWER_UP_NOT_COMPLETE)
    print_Str (" POWER_UP_NOT_COMPLETE,");
  if (initResp == OUT_OF_IDLE) // 0
    print_Str (" INIT_SUCCESS\n\r");
}

/*
 ******************************************************************************
 *                        "PRIVATE" FUNCTIONS DEFINITIONS
 ******************************************************************************
 */

/*
 * ----------------------------------------------------------------------------
 *                                          PRINT INITIALIZATION RESPONSE FLAGS
 * 
 * Description : Used by sd_SendCommand() to calculate the CRC7 for a command.
 * 
 * Arguments   : tca     The tx/cmd/arg bits used to calculate the CRC7.
 * 
 * Returns     : calculated CRC7 to be sent with the SD command. 
 * ----------------------------------------------------------------------------
 */
static uint8_t pvt_CRC7(uint64_t tca)
{
  // to test if division will take place during a given iteration.
  uint64_t test = 0x800000000000; 
  
  //
  // 0b10001001 (0x89) is divisor set in SD std to generate CRC7.
  // Byte 6 is initialized with this value.
  //
  uint64_t divisor = 0x890000000000;

  // initialize result with tx/cmd/arg portion of SD command.
  uint64_t result = tca;

  // calculate CRC7. 
  for (int cnt = 0; cnt < 40; cnt++)
  {
    if (result & test)
      result ^= divisor;
    divisor >>= 1;
    test >>= 1;
  }
  return result;
}
