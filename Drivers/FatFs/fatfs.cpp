/*
 * Arduino Wrapper Function Library for FatFs
 * (c) 2010, David Sirkin sirkin@stanford.edu
 *
 * FatFS by ChaN:
 * http://elm-chan.org/fsw/ff/00index_e.html
 */

#include "diskio.h"

#include "fatfs.h"

uint8_t _clkdivider;  //default spi clock 4MHz
uint8_t _ssi_module;  //default SSI module is 2 for compatibility with 430 booster packs


/*-------------------------------------------------------------------------

   Local Public Functions (In mm.c and diskio.h)

-------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* SPI Functions for diskio driver                  */
/*-----------------------------------------------------------------------*/

uint8_t FFS_SPI_RECEIVE (void)
{
	return SPI.transfer( 0xFF );
}

uint8_t FFS_SPI_SEND (const uint8_t data)
{
	return SPI.transfer( data );
}

void FFS_SPI_HIGH_SPEED (void)
{
	SPI.setClockDivider(_clkdivider);
}

void FFS_SPI_INIT_SPEED (void)
{
	SPI.setClockDivider(128);
}

/*-----------------------------------------------------------------------*/
/* Delay function for diskio driver                                      */
/*-----------------------------------------------------------------------*/

void FFS_DLY10U (void)
{
	delayMicroseconds(10);
}


/*-----------------------------------------------------------------------*/
/* Setup FS Structures and Register CS Pin                               */
/*-----------------------------------------------------------------------*/
FRESULT FFS::begin (
	unsigned char cs_pin	/* Pin to connect to CS */

)
{
	FRESULT res;

	res=begin(cs_pin, 2, 2);  //default clock divider 2 and SSI module 2

	return res;
}

FRESULT FFS::begin (
	unsigned char cs_pin,	/* Pin to connect to CS */
	uint8_t clkdivider
)
{
	FRESULT res;

	res=begin(cs_pin, clkdivider, 2);

	return res;
}



FRESULT FFS::begin (
	unsigned char cs_pin,	/* Pin to connect to CS */
	uint8_t clkdivider,
	uint8_t ssi_module
)
{
	FRESULT res;

	CS = cs_pin;
	pinMode(CS, OUTPUT);

	_clkdivider=clkdivider;
	_ssi_module=ssi_module;
#if defined(__LM4F120H5QR__) // StellarPad LM4F specific
	//SPI.begin(9);
	SPI.begin(CS);
	SPI.setModule(_ssi_module);
#else
	SPI.begin(CS);
	SPI.setModule(_ssi_module);
#endif

	attach_cs_pin(CS_LOW, CS_HIGH);
	attach_dly10u(DLY10U);
	attach_SPIdriver(SPI_RECEIVE, SPI_SEND, SPI_INIT_SPEED, SPI_HIGH_SPEED);
	//io_initialize();

	disk_init(0);

	res = mount(0, &fatfs_obj);

#if _FS_MINIMIZE <= 1
	res = opendir(&dir_obj, (const TCHAR *)"/");
#endif

	return res;
}


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS FFS::disk_init (
	unsigned char drv	/* Physical drive number (0) */
)
{
	return disk_initialize(drv);
}


/*-----------------------------------------------------------------------*/
/* Device Timer Interrupt Setup  (Platform dependent)                    */
/*-----------------------------------------------------------------------*/
/* Initialize and service a 10ms Output Compare A interrupt on Timer1    */

void FFS::io_init (void)
{
//	io_initialize();
}


/*-----------------------------------------------------------------------*/
/* Device Timer Interrupt Procedure (Platform Dependent)                 */
/*-----------------------------------------------------------------------*/
/* This function must be called in period of 10ms                        */

void FFS::timerproc (
	void
)
{
//	disk_timerproc();
}
