/*
 * File       : SD_SPI_BASE.C
 * Version    : 1.0
 * Target     : ATMega1280
 * Compiler   : AVR-GCC 9.3.0
 * Downloader : AVRDUDE 6.3
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020, 2021
 *  
 * Implementation of SD_SPI_BASE.H
 */

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
 *               sets the members of the CTV (Card Type and Version) struct
 *               instance. 
 *
 * Arguments   : ctv   - ptr to a CTV instance whose members will be set here.
 * 
 * Returns     : Initialization Error Response. This includes an Initialization
 *               Error Flag in bits 8 to 19 and the most recent R1 response in
 *               the lowest byte.
 * 
 * Warnings    : A CTV instance should ONLY be set by this function.
 * ----------------------------------------------------------------------------
 */
uint32_t sd_InitModeSPI(CTV *ctv)
{
  // 
  // initialize SPI port.
  //
  CS_SD_DDR |= 1 << CS_SD_DD;              // set chip select DD to input
  CS_SD_HIGH;                              // chip select is high (disabled)
  spi_MasterInit();                        // initialize SPI port.

  uint8_t r1;           // first byte ret by SD card in response to any cmd

  //
  // if prevSuccessFlag is set, then initialization has previously completed 
  // successfully so return. If not, proceed with rest of init function.
  //
  static uint8_t prevSuccessFlag;
  if (prevSuccessFlag)
    return OUT_OF_IDLE;

  sd_WaitSendDummySPI(80);                 // wait 80 SPI CCs for power up

  //
  // Step 1: GO_IDLE_STATE (CMD0)
  //
  CS_SD_LOW;
  sd_SendCommand(GO_IDLE_STATE, 0);         // send with arg = 0
  r1 = sd_GetR1();
  CS_SD_HIGH;
  if (r1 != IN_IDLE_STATE) 
    return (FAILED_GO_IDLE_STATE | r1);

  //
  // Step 2: SEND_IF_COND (CMD8)
  //
  uint8_t r7[R7_BYTE_LEN];                  // R7 response is only for CMD8

  CS_SD_LOW;
  sd_SendCommand(SEND_IF_COND, SEND_IF_COND_ARG);

  // Get R7 response.
  r7[R7_R1_RESP_BYTE] = sd_GetR1();              // First R7 byte is the R1 response
  r7[R7_CMD_VERS_BYTE] = sd_ReceiveByteSPI();    // not used
  r7[R7_RSRVD_BYTE] = sd_ReceiveByteSPI();       // not used
  r7[R7_VOLT_RNG_ACPTD_BYTE] = sd_ReceiveByteSPI();
  r7[R7_CHK_PTRN_ECHO_BYTE] = sd_ReceiveByteSPI();
  CS_SD_HIGH;
  if (r7[R7_R1_RESP_BYTE] == (ILLEGAL_COMMAND | IN_IDLE_STATE)) 
    ctv->version = VERSION_1;
  else if (r7[R7_R1_RESP_BYTE] == IN_IDLE_STATE) 
  {
    ctv->version = VERSION_2;
    if (r7[R7_VOLT_RNG_ACPTD_BYTE] != VOLT_RANGE_SUPPORTED 
        || r7[R7_CHK_PTRN_ECHO_BYTE] != CHECK_PATTERN)
      return (FAILED_SEND_IF_COND | UNSUPPORTED_CARD_TYPE | r7[R7_R1_RESP_BYTE]);
  }
  else  
    return (FAILED_SEND_IF_COND | r7[R7_R1_RESP_BYTE]);
  
  //
  // Step 3: CRC_ON_OFF (CMD59)
  //
  CS_SD_LOW;
  sd_SendCommand(CRC_ON_OFF, CRC_OFF_ARG);
  r1 = sd_GetR1();
  CS_SD_HIGH;
  if (r1 != IN_IDLE_STATE) 
    return (FAILED_CRC_ON_OFF | r1);

  //
  // Step 4: SD_SEND_OP_COND (ACMD41)
  //
  // The argument of SD_SEND_OP_COND indicates card capacity supported by the 
  // host. This is determined by the setting of macro HOST_CAPACITY_SUPPORT.
  // Since SD_SEND_OP_COND is an ACMD type, the APP_CMD, must first be sent to
  // signal to the SD card that the next command is type ACMD. This process of
  // send APP_CMD then SD_SEND_OP_COND repeats until the R1 response to 
  // SD_SEND_OP_COND signals the card is no longer in the idle state or timeout
  // limit has been reached.
  //
  uint8_t  timeout = 0;
  do
  {
    CS_SD_LOW;
    sd_SendCommand(APP_CMD, 0);             // arg is 0 for this command
    r1 = sd_GetR1();
    CS_SD_HIGH;
    if (r1 != IN_IDLE_STATE) 
      return (FAILED_APP_CMD | r1);
    CS_SD_LOW;
    sd_SendCommand(SD_SEND_OP_COND, ACMD41_HCS_ARG);
    r1 = sd_GetR1();
    CS_SD_HIGH;
    if (r1 > IN_IDLE_STATE)
      return (FAILED_SD_SEND_OP_COND | r1);
    if (++timeout >= TIMEOUT_LIMIT && r1 != OUT_OF_IDLE)
      return (FAILED_SD_SEND_OP_COND | OUT_OF_IDLE_TIMEOUT | r1);
  }
  while (r1 & IN_IDLE_STATE);

  //
  // Step 5: READ_OCR (CMD 58)
  // The final init step is to read Card Capacity Support bit (CCS) of the OCR
  //

  // OCR Variables
  uint8_t  ocr;
  uint16_t vra;                             // OCR voltage range accepted
  
  CS_SD_LOW;
  sd_SendCommand(READ_OCR, 0);              // arg is 0 for this command
  r1 = sd_GetR1();
  if (r1 != OUT_OF_IDLE)
    return (FAILED_READ_OCR | r1);

  ocr = sd_ReceiveByteSPI();                // load MSByte of OCR

  // power up status
  if (!(ocr & POWER_UP_BIT_MASK))
  {
    CS_SD_HIGH;
    return (POWER_UP_NOT_COMPLETE | r1);
  }
  
  // CCS is needed to set card type.
  if (ocr & CCS_BIT_MASK)
    ctv->type = SDHC;
  else 
    ctv->type = SDSC;

  // load two bytes for the vra
  vra = sd_ReceiveByteSPI();
  vra <<= 8;
  vra |= sd_ReceiveByteSPI();

  if (ocr & (UHSII_BIT_MASK | CO2T_BIT_MASK | S18A_BIT_MASK) 
      || vra != VRA_OCR_MASK)
  {
    CS_SD_HIGH;
    return (FAILED_READ_OCR | UNSUPPORTED_CARD_TYPE | r1);
  }

  // Initialization success
  CS_SD_HIGH;
  prevSuccessFlag = 1;
  return OUT_OF_IDLE;
}

/*
 * ----------------------------------------------------------------------------
 *                                                                    SEND BYTE
 * 
 * Description : Sends a single 8-bit byte to the SD card via the SPI port.
 * 
 * Arguments   : byte   - 8-bit byte to be sent to the SD Card via SPI.
 * 
 * Returns     : void
 * 
 * Notes       : 1) Call as many times as required to send the complete data 
 *                  packet, token, command, etc...
 *               2) This function and sd_ReceiveByteSPI(), are the only direct
 *                  SPI interfacing functions in the SD card module.
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
 * Description : Receives and returns single byte from the SD card via SPI.
 * 
 * Arguments   : void
 * 
 * Returns     : 8-bit byte received from the SD card.
 * 
 * Notes       : 1) Call as many times as necessary to get the complete data
 *                  packet, token, error response, etc... from the SD card.
 *               2) This function and sd_SendByteSPI(), are the only direct
 *                  SPI interfacing functions in the SD card module.
 * ----------------------------------------------------------------------------
 */
uint8_t sd_ReceiveByteSPI(void)
{
  sd_SendByteSPI(DMY_TKN);             // send dummy byte to initiate response
  return spi_MasterReceive();          // return SD card's response
}

/*
 * ----------------------------------------------------------------------------
 *                                                                 SEND COMMAND
 * 
 * Description : Send a command and argument to the SD Card.
 * 
 * Arguments   : cmd   - SD Card command. See sd_spi_car.h.
 *               arg   - 32-bit argument to be sent with the SD command.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */
void sd_SendCommand(uint8_t cmd, uint32_t arg)
{
  // Found forcing some delay between commands can improve stability/behavrior.
  sd_WaitSendDummySPI(80);
                           
  // 
  // Construct the command / argument packet to be sent to the SD card. The
  // form, from MSB to LSB is:
  // TX_CMD (2b) | CMD (6b) | ARG (32b) | CRC7 (7b) | STOP_BIT (1b)
  //
  uint64_t tcacs = (uint64_t)(TX_CMD_BITS | cmd) << 40;   // load tx and cmd
  tcacs |= (uint64_t)arg << 8;
  tcacs |= pvt_CRC7(tcacs);
  tcacs |= STOP_BIT;

  // Send cmd / arg to SD Card via SPI port. 8-bits at a time.
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
 * Notes       : 1) always call immediately after sd_SendCommand().
 *               2) pass the return value to sd_PrintR1() to print R1 response.
 *               3) if R1_TIMEOUT is returned, then the SD Card did not return
 *                  a response.
 * ----------------------------------------------------------------------------
 */
uint8_t sd_GetR1(void)
{
  uint8_t r1;
  
  // loop until SPDR has new values (i.e != dummy token or TO limit reached.
  for (uint8_t timeout = 0; (r1 = sd_ReceiveByteSPI()) == DMY_TKN; ++timeout)
    if(timeout >= TIMEOUT_LIMIT) 
      return R1_TIMEOUT;
  return r1;
}

/*
 * ----------------------------------------------------------------------------
 *                                                      PRINT R1 RESPONSE FLAGS
 * 
 * Description : Prints the R1 response flag(s) returned by sd_GetR1().
 * 
 * Arguments   : r1   - The R1 response flag(s) byte returned by sd_GetR1().
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
 * Description : Prints Initialization Error Flag portion of the response 
 *               returned by sd_InitModeSPI.
 * 
 * Arguments   : initResp   - The Initialization Error Response returned by the
 *                            initialization routine, sd_InitModeSPI.
 * 
 * Returns     : void
 * 
 * Notes       : This will only interpret bits 8 to 19 of the sd_InitModeSPI 
 *               function's returned value. Though the entire returned value
 *               can be passed to the function, bits 0 to 7 will be ignored
 *               as these are the R1 Response portion of the Initialization
 *               Response. To read the R1 portion of initResp, pass it to 
 *               sd_PrintR1().
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
 * ----------------------------------------------------------------------------
 *                                              WAIT SPECIFIED SPI CLOCK CYCLES
 * 
 * Description : Used to wait a specified number of SPI clock cycles. While
 *               doing so, it sends all 1's (DMY_TKN) on the SPI port.
 * 
 * Arguments   : clckCycles   - min num of clock cycles to wait.
 * 
 * Returns     : void
 * ----------------------------------------------------------------------------
 */
void sd_WaitSendDummySPI(uint16_t clckCycles)
{
  for (uint8_t waitCnt = 0; waitCnt < clckCycles / SPI_REG_BIT_LEN; ++waitCnt)
    sd_SendByteSPI(DMY_TKN);
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
 * Arguments   : tca   - the tx/cmd/arg bits used to calculate the CRC7.
 * 
 * Returns     : calculated CRC7 to be sent with the SD command. 
 * ----------------------------------------------------------------------------
 */
static uint8_t pvt_CRC7(uint64_t tca)
{
  // to test if division will take place during a given calc iteration.
  uint64_t test = 0x800000000000; 
  
  //
  // 0b10001001 (0x89) is divisor set in SD std to generate CRC7.
  // Byte 6 is initialized with this value.
  //
  uint64_t divisor = 0x890000000000;

  // initialize result with tx/cmd/arg portion of SD command.
  uint64_t result = tca;

  // calculate CRC7. 
  for (uint8_t calc = 0; calc < 40; ++calc)
  {
    if (result & test)
      result ^= divisor;
    divisor >>= 1;
    test >>= 1;
  }
  return result;
}
