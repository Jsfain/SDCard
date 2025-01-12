/*
 * File       : SD_SPI_CAR.H
 * Version    : 1.0
 * License    : GNU GPLv3
 * Author     : Joshua Fain
 * Copyright (c) 2020 - 2024
 * 
 * Macros for SD Card Commands, Arguments, Responses available in SPI mode,
 * along with some other related macros.
 * 
 * This file should only be included from sd_spi_base.h
 */

#ifndef SD_SPI_CAR_H
#define SD_SPI_CAR_H

/*
 ******************************************************************************
 *                                COMMANDS
 ******************************************************************************
 */
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
// Application Specific Commands. To use, first call APP_CMD (CMD55).
#define SD_STATUS                   13      //ACMD13
#define SEND_NUM_WR_BLOCKS          22      //ACMD22
#define SET_WR_BLK_ERASE_COUNT      23      //ACMD23
#define SD_SEND_OP_COND             41      //ACMD41
#define SET_CLR_CARD_DETECT         42      //ACMD42
#define SEND_SCR                    51      //ACMD51

/*
 ******************************************************************************
 *                                CMD ARGUMENTS
 ******************************************************************************
 */

//
// Some constant cmd args used in this current implementation.
//

//
// SEND_IF_COND arguments
//
#define VOLT_RANGE_SUPPORTED  0x01 // 0x01 supports 2.7 to 3.6V 
#define CHECK_PATTERN         0xAA // arbitrary val. sent to/retnd by SD card
#define SEND_IF_COND_ARG      (VOLT_RANGE_SUPPORTED << 8) | CHECK_PATTERN

//
// CRC_ON_OFF arguments
//
#define CRC_ON_ARG      1
#define CRC_OFF_ARG     0

//
// SD_SEND_OP_COND arguments
//
#if HOST_CAPACITY_SUPPORT == SDHC
#define ACMD41_HCS_ARG 0x40000000
#else
#define ACMD41_HCS_ARG 0
#endif

/*
 ******************************************************************************
 *                                CMD RESPONSES
 ******************************************************************************
 */

/* 
 * ----------------------------------------------------------------------------
 *                                                            R1 RESPONSE FLAGS
 * 
 * Description : The R1 response flags returned by sd_GetR1().
 * 
 * Notes       : 1) With the exception of R1_TIMEOUT, these flags correspond to
 *                  the first byte returned by the SD card in response to any 
 *                  command.
 *               2) R1_TIMEOUT will be set in the sd_GetR1() return value if 
 *                  the card does not send an R1 response after set amount
 *                  of attempts/time.
 * ----------------------------------------------------------------------------
 */
#define OUT_OF_IDLE             0x00        // no R1 error
#define IN_IDLE_STATE           0x01
#define ERASE_RESET             0x02
#define ILLEGAL_COMMAND         0x04
#define COM_CRC_ERROR           0x08
#define ERASE_SEQUENCE_ERROR    0x10
#define ADDRESS_ERROR           0x20
#define PARAMETER_ERROR         0x40
#define R1_TIMEOUT              0x80        // not R1 std setting

#define R1_MASK                 0x7F        // all std R1 bits set

/* 
 * ----------------------------------------------------------------------------
 *                                                                  R7 RESPONSE
 * 
 * Description : The R7 response is a 5 bytes long response returned by the SD 
 *               card in response to the SEND_IF_COND command (CMD8).
 * ----------------------------------------------------------------------------
 */
#define R7_BYTE_LEN               5         // byte length of R7 response

// These are the order of the R7 response bytes returned by the SD card.
#define R7_R1_RESP_BYTE           0
#define R7_CMD_VERS_BYTE          1  
#define R7_RSRVD_BYTE             2
#define R7_VOLT_RNG_ACPTD_BYTE    3
#define R7_CHK_PTRN_ECHO_BYTE     4

#endif //SD_SPI_CAR_H
