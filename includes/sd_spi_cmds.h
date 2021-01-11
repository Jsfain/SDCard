/*
Header to define the SD card commands available in SPI mode.

File    : SD_SPI_CMDS.H
Version : 0.0.0.1 
Author  : Joshua Fain
Target  : ATMega1280
License : MIT
Copyright (c) 2020-2021
*/


#ifndef SD_SPI_CMDS_H
#define SD_SPI_CMDS_H

/*! \file sd_spi_cmds.h */
/*! \defgroup sd_spi_cmds <sd_spi_cmds.h>
 *  \brief **SD Card SPI Mode commands** 
 *  \attention Application commands require sending APP_CMD (55) to the SD card
 *  first to signal that the next command is an app command. 
 */


/*
*******************************************************************************
*******************************************************************************
 *                     
 *                                   MACROS
 *  
*******************************************************************************
*******************************************************************************
*/

// ----------------------------------------------------------------------------
//                                                             SD Card Commands
/*! \ingroup sd_spi_cmds */
#define GO_IDLE_STATE               0       //CMD0
/*! \ingroup sd_spi_cmds */
#define SEND_OP_COND                1       //CMD1
/*! \ingroup sd_spi_cmds */
#define SWITCH_FUNC                 6       //CMD6
/*! \ingroup sd_spi_cmds */
#define SEND_IF_COND                8       //CMD8
/*! \ingroup sd_spi_cmds */ 
#define SEND_CSD                    9       //CMD9
/*! \ingroup sd_spi_cmds */ 
#define SEND_CID                    10      //CMD10
/*! \ingroup sd_spi_cmds */ 
#define STOP_TRANSMISSION           12      //CMD12
/*! \ingroup sd_spi_cmds */
#define SEND_STATUS                 13      //CMD13
/*! \ingroup sd_spi_cmds */
#define SET_BLOCKLEN                16      //CMD16
/*! \ingroup sd_spi_cmds */
#define READ_SINGLE_BLOCK           17      //CMD17
/*! \ingroup sd_spi_cmds */
#define READ_MULTIPLE_BLOCK         18      //CMD18
/*! \ingroup sd_spi_cmds */
#define WRITE_BLOCK                 24      //CMD24
/*! \ingroup sd_spi_cmds */
#define WRITE_MULTIPLE_BLOCK        25      //CMD25
/*! \ingroup sd_spi_cmds */
#define PROGRAM_CSD                 27      //CMD27
/*! \ingroup sd_spi_cmds */
#define SET_WRITE_PROT              28      //CMD28
/*! \ingroup sd_spi_cmds */
#define CLR_WRITE_PROT              29      //CMD29
/*! \ingroup sd_spi_cmds */
#define SEND_WRITE_PROT             30      //CMD30
/*! \ingroup sd_spi_cmds */
#define ERASE_WR_BLK_START_ADDR     32      //CMD32
/*! \ingroup sd_spi_cmds */
#define ERASE_WR_BLK_END_ADDR       33      //CMD33
/*! \ingroup sd_spi_cmds */
#define ERASE                       38      //CMD38
/*! \ingroup sd_spi_cmds */
#define LOCK_UNLOCK                 42      //CMD42
/*! \ingroup sd_spi_cmds */
#define APP_CMD                     55      //CMD55
/*! \ingroup sd_spi_cmds */
#define GEN_CMD                     56      //CMD56
/*! \ingroup sd_spi_cmds */
#define READ_OCR                    58      //CMD58
/*! \ingroup sd_spi_cmds */
#define CRC_ON_OFF                  59      //CMD59
// Application Specific Commands. To activate, first call APP_CMD.
/*! \ingroup sd_spi_cmds 
 *  \brief app command
 *  \note send APP_CMD first before sending */
#define SD_STATUS                   13      //ACMD13
/*! \ingroup sd_spi_cmds 
 *  \brief app command
 *  \note send APP_CMD first before sending */
#define SEND_NUM_WR_BLOCKS          22      //ACMD22
/*! \ingroup sd_spi_cmds 
 *  \brief app command
 *  \note send APP_CMD first before sending */
#define SET_WR_BLK_ERASE_COUNT      23      //ACMD23
/*! \ingroup sd_spi_cmds 
 *  \brief app command
 *  \note send APP_CMD first before sending */
#define SD_SEND_OP_COND             41      //ACMD41
/*! \ingroup sd_spi_cmds 
 *  \brief app command
 *  \note send APP_CMD first before sending */
#define SET_CLR_CARD_DETECT         42      //ACMD42
/*! \ingroup sd_spi_cmds 
 *  \brief app command
 *  \note send APP_CMD first before sending */
#define SEND_SCR                    51      //ACMD51

#endif //SD_SPI_CMDS_H