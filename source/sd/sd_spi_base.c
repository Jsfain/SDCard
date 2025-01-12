/*
 * File       : SD_SPI_BASE.C
 * Version    : 1.0
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020 - 2025
 * 
 * SD_SPI_BASE.C defines the functions from SD_SPI_BASE.H. These are the basic
 * functions required to access an SD card in SPI mode.
 */

#include <stdint.h>
#include "sd_spi_base.h"

/*
 ******************************************************************************
 *                        "PRIVATE" FUNCTION PROTOTYPES
 ******************************************************************************
 */

static uint8_t pvt_CRC7(uint64_t tca);           // returns the CRC7 checksum


/*
 ******************************************************************************
 *                                   FUNCTIONS   
 ******************************************************************************
 */

/*
 * ----------------------------------------------------------------------------
 *                                              SPI MODE SD CARD INITIALIZATION
 *
 * Description : Implements the SD Card's SPI mode initialization routine and 
 *               sets the members of the CTV (Card Type and Version) struct
 *               instance. 
 *
 * Arguments   : ctv - pointer to CTV instance whose members are set during
 *                     the initialization routine.
 * 
 * Returns     : Initialization Response. This includes any Initialization
 *               Error Flags set in bits 8 to 16 and the most recent R1 
 *               response in the lowest byte.
 * 
 * Warning     : Any instance of CTV should ONLY be set by this function.
 * ----------------------------------------------------------------------------
 */
uint32_t sd_InitSpiMode(CTV *ctv)
{
  uint8_t r1;                     // for r1 response

  sd_InitMasterModeSPI();                   // init SPI port in master mode
  sd_WaitSpiClkCyclesSPI(POWERUP_WAIT);     // wait for power up to complete

  //
  // Step 1: GO_IDLE_STATE (CMD0)
  //
  CS_ASSERT;
  sd_SendCommand(GO_IDLE_STATE, 0);         // send with arg = 0
  r1 = sd_GetR1();
  CS_DEASSERT;
  if (r1 != IN_IDLE_STATE) 
    return (FAILED_GO_IDLE_STATE | r1);

  //
  // Step 2: SEND_IF_COND (CMD8)
  //
  uint8_t r7[R7_BYTE_LEN];                  // R7 response is only for CMD8

  CS_ASSERT;
  sd_SendCommand(SEND_IF_COND, SEND_IF_COND_ARG);
  // Get R7 response.
  r7[R7_R1_RESP_BYTE] = sd_GetR1();         // First R7 byte is the R1 response
  r7[R7_CMD_VERS_BYTE] = sd_ReceiveByteFromSD();    // not used
  r7[R7_RSRVD_BYTE] = sd_ReceiveByteFromSD();       // not used
  r7[R7_VOLT_RNG_ACPTD_BYTE] = sd_ReceiveByteFromSD();
  r7[R7_CHK_PTRN_ECHO_BYTE] = sd_ReceiveByteFromSD();
  CS_DEASSERT;
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
  CS_ASSERT;
  sd_SendCommand(CRC_ON_OFF, CRC_OFF_ARG);
  r1 = sd_GetR1();
  CS_DEASSERT;
  if (r1 != IN_IDLE_STATE) 
    return (FAILED_CRC_ON_OFF | r1);

  //
  // Step 4: SD_SEND_OP_COND (ACMD41)
  //
  // SD_SEND_OP_COND is type ACMD, thus APP_CMD must first be sent to signal 
  // that the next incoming command is type ACMD. The SD_SEND_OP_COND argument
  // indicates the card capacity supported by the host (HOST_CAPACITY_SUPPORT).
  // The process repeats until the SD_SEND_OP_COND R1 response shows card is no
  // longer in the idle state or the attempt/timeout limit has been reached.
  //
  uint8_t  attempts = 0;
  do
  {
    CS_ASSERT;
    sd_SendCommand(APP_CMD, 0);             // arg is 0 for this command
    r1 = sd_GetR1();
    CS_DEASSERT;
    if (r1 != IN_IDLE_STATE) 
      return (FAILED_APP_CMD | r1);
    CS_ASSERT;
    sd_SendCommand(SD_SEND_OP_COND, ACMD41_HCS_ARG);
    r1 = sd_GetR1();
    CS_DEASSERT;
    if (r1 > IN_IDLE_STATE)
      return (FAILED_SD_SEND_OP_COND | r1);
    if (++attempts >= MAX_ATTEMPTS && r1 != OUT_OF_IDLE)
      return (FAILED_SD_SEND_OP_COND | OUT_OF_IDLE_TIMEOUT | r1);
  }
  while (r1 & IN_IDLE_STATE);

  //
  // Step 5: READ_OCR (CMD 58)
  // 
  // The final init step reads the OCR to verify the card type and voltage
  // range are supported by the host. Additionally, Card Capacity Support (CCS)
  // bit of the OCR is used to set the type member of the CTV instance, critical
  // for correct addressing of data on the card.
  //
  uint8_t  ocr;
  uint16_t vra;                             // OCR voltage range accepted
  
  CS_ASSERT;
  sd_SendCommand(READ_OCR, 0);              // arg is 0 for this command
  r1 = sd_GetR1();
  if (r1 != OUT_OF_IDLE)
    return (FAILED_READ_OCR | r1);

  ocr = sd_ReceiveByteFromSD();             // load MSByte of OCR

  // power up status
  if (!(ocr & POWER_UP_BIT_MASK))
  {
    CS_DEASSERT;
    return (POWER_UP_NOT_COMPLETE | r1);
  }
  
  // CCS is used to set card type
  if (ocr & CCS_BIT_MASK)
    ctv->type = SDHC;
  else 
    ctv->type = SDSC;

  // load two bytes for the vra
  vra = sd_ReceiveByteFromSD();
  vra <<= 8;
  vra |= sd_ReceiveByteFromSD();

  // verify voltage range of card and unsupported card type is not used
  if (ocr & (UHSII_BIT_MASK | CO2T_BIT_MASK | S18A_BIT_MASK) 
      || vra != VRA_OCR_MASK)
  {
    CS_DEASSERT;
    return (FAILED_READ_OCR | UNSUPPORTED_CARD_TYPE | r1);
  }

  // initialization success
  CS_DEASSERT;
  return OUT_OF_IDLE;
}

/*
 * ----------------------------------------------------------------------------
 *                                                                    SEND BYTE
 * 
 * Description : Sends a single 8-bit byte to the SD card via the SPI port.
 * 
 * Arguments   : byte   - byte to be sent to the SD Card via SPI.
 * 
 * Note        : Call this function as many times as necessary to send the 
 *               complete data packet, token, command, etc...
 * ----------------------------------------------------------------------------
 */
void sd_SendByteToSD(uint8_t byte)
{
  sd_TransmitByteSPI(byte);       // sends byte via SPI port.
}

/*
 * ----------------------------------------------------------------------------
 *                                                                 RECEIVE BYTE
 * 
 * Description : Receive a single 8-bit byte from the SD card via the SPI port.
 * 
 * Returns     : byte received from the SD card.
 * 
 * Note        : Call this function as many times as necessary to retrieve the
 *               complete data packet, token, error response, etc...
 * ----------------------------------------------------------------------------
 */
uint8_t sd_ReceiveByteFromSD(void)
{
  return sd_ReceiveByteSPI();
}

/*
 * ----------------------------------------------------------------------------
 *                                                                 SEND COMMAND
 * 
 * Description : Send a command and argument to the SD Card. See sd_spi_car.h 
 *               for the list of command and argument macros.
 * 
 * Arguments   : cmd   - SD Card command.
 *               arg   - 32-bit argument to be sent with the SD command.
 * ----------------------------------------------------------------------------
 */
void sd_SendCommand(uint8_t cmd, uint32_t arg)
{
  // Forcing a wait period between commands improved stability/behavior.
  sd_WaitSpiClkCyclesSPI(CMD_WAIT);
                           
  // 
  // Construct the command/argument packet to be sent to the SD card. The form
  // of the packet from MSB to LSB is:
  // TX_CMD (2b) | CMD (6b) | ARG (32b) | CRC7 (7b) | STOP_BIT (1b)
  //
  uint64_t tcacs = (uint64_t)(TX_CMD_BITS | cmd) << 40;   // load tx and cmd
  tcacs |= (uint64_t)arg << 8;
  tcacs |= pvt_CRC7(tcacs);                 // calculate and load CRC7
  tcacs |= STOP_BIT;

  // Send cmd/arg to SD Card via SPI port 8-bits at a time
  sd_SendByteToSD((uint8_t)(tcacs >> 40));
  sd_SendByteToSD((uint8_t)(tcacs >> 32));
  sd_SendByteToSD((uint8_t)(tcacs >> 24));
  sd_SendByteToSD((uint8_t)(tcacs >> 16));
  sd_SendByteToSD((uint8_t)(tcacs >> 8));
  sd_SendByteToSD((uint8_t)(tcacs));
}

/*
 * ----------------------------------------------------------------------------
 *                                                              GET R1 RESPONSE
 * 
 * Description : Used to retrieve the R1 response from the SD card immediately 
 *               after a command is sent. It will return once a valid R1 value
 *               has been retrieved or MAX_ATTEMPT limit reached.
 * 
 * Returns     : R1 response (see sd_spi_car.h) or R1_TIMEOUT error.
 * 
 * Notes       : 1) always call immediately after calling sd_SendCommand.
 *               2) only call immediately after calling sd_SendCommand.
 *               3) if R1_TIMEOUT is returned, then the SD Card did not return
 *                  a valid R1 response within the MAX_ATTEMPT limit.
 *               4) a valid R1 response is of the form 0b0XXXXXXX.
 * ----------------------------------------------------------------------------
 */
uint8_t sd_GetR1(void)
{
  uint8_t r1;

  // loop until valid R1 response received or attempt limit reached.
  for (uint8_t att = 0; (r1 = sd_ReceiveByteFromSD()) & ~R1_MASK; ++att)
    if(att >= MAX_ATTEMPTS) 
      return R1_TIMEOUT;
  return r1;
}

/*
 ******************************************************************************
 *                        "PRIVATE" FUNCTIONS DEFINITIONS
 ******************************************************************************
 */

/*
 * ----------------------------------------------------------------------------
 *                                                      CALCULATE CRC7 CHECKSUM
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
