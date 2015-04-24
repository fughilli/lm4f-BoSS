/*
 * Arduino Wrapper Function Library for FatFs
 * (c) 2010, David Sirkin sirkin@stanford.edu
 *
 * FatFS by ChaN:
 * http://elm-chan.org/fsw/ff/00index_e.html
 */

#ifndef FFS_h
#define FFS_h

#include "ff.h"
#include "diskio.h"
#include "ffconf.h"
#include "integer.h"

static FATFS fatfs_obj;
static DIR dir_obj;
static FIL fil_obj;
static FILINFO fno;

static unsigned char CS;

static void FFS_CS_LOW(void);
static void FFS_CS_HIGH(void);
static void FFS_DLY10U(void);
static uint8_t FFS_SPI_RECEIVE(void);
static uint8_t FFS_SPI_SEND(const uint8_t);
static void FFS_SPI_INIT_SPEED(void);
static void FFS_SPI_HIGH_SPEED(void);

/*--------------------------------------------------------------*/
/* Functions for Local Disk Control                             */
/*--------------------------------------------------------------*/

FRESULT FFS_ffs_begin(unsigned char);
FRESULT FFS_begin(unsigned char, uint8_t);
FRESULT FFS_begin(unsigned char, uint8_t, uint8_t);
DSTATUS FFS_disk_init(unsigned char);
void FFS_io_init(void);
void FFS_timerproc(void);

#endif

