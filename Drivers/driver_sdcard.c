/*
 * driver_sdcard.c
 *
 *  Created on: Apr 23, 2015
 *      Author: Kevin
 */

#include "driver_sdcard.h"
#include "../fast_utils.h"

#include "driverlib/gpio.h"
#include "inc/hw_gpio.h"

#include "inc/hw_memmap.h"

#include "driverlib/sysctl.h"

#include "FatFs/ff.h"
#include "FatFs/diskio.h"
#include "FatFs/integer.h"

#include "../debug_serial.h"

#include "SPI/SPI.h"

#define SDCARD_SPI_MODULE (3)

#define SDCARD_SS_PERIPH (SYSCTL_PERIPH_GPIOD)
#define SDCARD_SS_PORTBASE (GPIO_PORTD_BASE)
#define SDCARD_SS_PINMASK (1 << 1)

#define SDCARD_MAX_OPEN_FILES (8)

#define SDCARD_FD_VALID(_fd_) (((_fd_) >= 0) && ((_fd_) < MAX_FILES) && (file_table[(_fd_)].funmap == &sdcard_file_funmap))

// Table for storing the open FIL structs
// These will be associated with fd's through the
// data member of the fd table
typedef struct
{
	FIL file;
	bool used;
} sdcard_open_file_t;

sdcard_open_file_t sdcard_open_files[SDCARD_MAX_OPEN_FILES];

int32_t sdcard_get_free_open_file_struct()
{
	int32_t i;
	for(i = 0; i < SDCARD_MAX_OPEN_FILES; i++)
	{
		if(!sdcard_open_files[i].used)
			return i;
	}
	return -1;
}

/*
 * Allocates a file_table entry and a matching sdcard_open_files entry for an open sdcard file
 */
fd_t sdcard_alloc_fd()
{
	fd_t fd;
	int32_t fofs_idx;
	if((fd = ftable_getfree()) != FD_INVALID && (fofs_idx = sdcard_get_free_open_file_struct()) != -1)
	{
		file_table[fd].funmap = &sdcard_file_funmap;
		file_table[fd].fdata = &sdcard_open_files[fofs_idx];
		sdcard_open_files[fofs_idx].used = true;
		return fd;
	}

	return FD_INVALID;
}

/*
 * Look up an open file struct pointer from a fd
 */
sdcard_open_file_t* sdcard_ofsp_from_fd(fd_t fd)
{
	if(!SDCARD_FD_VALID(fd))
		return NULL;

	return (sdcard_open_file_t*)(file_table[fd].fdata);
}

/*
 * Free an sdcard fd and the associated open file struct
 */
void sdcard_free_fd(fd_t fd)
{
	if(!SDCARD_FD_VALID(fd))
		return;

	sdcard_ofsp_from_fd(fd)->used = false;
	ftable_free(fd);
}

typedef struct
{
	FATFS fatfs;
	FILINFO finfo;
	DIR dinfo;
} sdcard_fsys_data_t;

sdcard_fsys_data_t sdcard_data;

// Callback for setting the slave select pin high
static void sdcard_ss_low()
{
	GPIOPinWrite(SDCARD_SS_PORTBASE, SDCARD_SS_PINMASK, 0);
}

// Callback for setting the slave select pin low
static void sdcard_ss_high()
{
	GPIOPinWrite(SDCARD_SS_PORTBASE, SDCARD_SS_PINMASK, SDCARD_SS_PINMASK);
}

// Callback for delaying 10us
static void sdcard_delay_10us()
{
	SysCtlDelay(100);
}

static BYTE sdcard_spi_receive()
{
	return SPI_transfer(SDCARD_SPI_MODULE, 0xFF);
}

static BYTE sdcard_spi_send(uint8_t data)
{
	return SPI_transfer(SDCARD_SPI_MODULE, data);
}

static void sdcard_spi_init_speed()
{
	SPI_setClockDivider(SDCARD_SPI_MODULE, 128);
}

static void sdcard_spi_high_speed()
{
	SPI_setClockDivider(SDCARD_SPI_MODULE, 2);
}

void sdcard_mount()
{
	SysCtlPeripheralEnable(SDCARD_SS_PERIPH);

	GPIOPinTypeGPIOOutput(SDCARD_SS_PORTBASE, SDCARD_SS_PINMASK);

	sdcard_ss_low();

	SPI_begin(SDCARD_SPI_MODULE);

	attach_cs_pin(sdcard_ss_low, sdcard_ss_high);
	attach_dly10u(sdcard_delay_10us);
	attach_SPIdriver(sdcard_spi_receive, sdcard_spi_send, sdcard_spi_init_speed, sdcard_spi_high_speed);

	disk_initialize(0);

	f_mount(0, &sdcard_data.fatfs);

	f_opendir(&sdcard_data.dinfo, (const TCHAR*)"/");

//	do{
//		f_readdir(&sdcard_data.dinfo, &sdcard_data.finfo);
//		if((sdcard_data.finfo.fattrib & AM_DIR) != AM_DIR)
//		{
//			Serial_puts(UART_DEBUG_MODULE, sdcard_data.finfo.fname, 100);
//			Serial_puts(UART_DEBUG_MODULE, "\r\n", 2);
//		}
//	} while(sdcard_data.finfo.fname[0] != 0);
}

void sdcard_unmount(fsys_t fsys)
{

}

bool sdcard_listdir(char* fnamebuf, size_t fnamebuflen)
{
	FRESULT res = f_readdir(&sdcard_data.dinfo, &sdcard_data.finfo);
	//(sdcard_data.finfo.fattrib & AM_DIR) != AM_DIR
	if(res == FR_OK && sdcard_data.finfo.fname[0] != 0)
	{
		fast_strncpy(fnamebuf, sdcard_data.finfo.fname, fnamebuflen);
		fnamebuf[fnamebuflen - 1] = 0;	// Ensure null-termination
		return true;
	}
	// Rewind the directory
	f_readdir(&sdcard_data.dinfo, NULL);
	return false;
}

void sdcard_rwdir()
{
	// Rewind the directory
	f_readdir(&sdcard_data.dinfo, NULL);
}

bool sdcard_unlink(const char* fname)
{
	return (f_unlink((const TCHAR*) fname) == FR_OK);
}

fd_t sdcard_open(const char* fname, fmode_t mode, fflags_t flags)
{
	// TODO: ensure that there are no side effects on failure to allocate/open
	FIL tempfil;
	BYTE modemask = 0;

	/*TODO: insert flags here; should map to fsys flags for DankOS*/
	if(mode & FMODE_R)
		modemask |= FA_READ;
	if(mode & FMODE_W)
		modemask |= FA_WRITE;
	if(flags & FFLAG_CREAT)
		modemask |= FA_CREATE_ALWAYS;
	else
		modemask |= FA_OPEN_EXISTING;

	fd_t fd = sdcard_alloc_fd();
	if (fd != FD_INVALID)
	{
		file_table[fd].sysflags = flags & FFLAG_SYS_MASK;
		sdcard_open_file_t* ofp = sdcard_ofsp_from_fd(fd);
		if (ofp)
		{
			FRESULT res =
					f_open(&tempfil,
							(const TCHAR*) fname,
							modemask
							);

			if (res == FR_OK || res == FR_EXIST)
			{
				fast_memcpy(&ofp->file, &tempfil, sizeof(FIL));

				return fd;
			}
		}
	}

	sdcard_free_fd(fd);
	return FD_INVALID;
}

bool sdcard_chdir(const char* dirname)
{
	FRESULT res;
	DIR tdirinfo;
	if((res = f_opendir(&tdirinfo, (const TCHAR*) dirname)) != FR_OK)
		return false;

	if((res = f_chdir((const TCHAR*) dirname)) != FR_OK)
		return false;

	fast_memcpy(&sdcard_data.dinfo, &tdirinfo, sizeof(DIR));

	return true;
}

bool sdcard_mkdir(const char* dirname)
{
	FRESULT res;
	if((res = f_mkdir((const TCHAR*) dirname)) != FR_OK)
		return false;

	return true;
}

void sdcard_file_close(fd_t fd)
{
	sdcard_open_file_t* ofp = sdcard_ofsp_from_fd(fd);

	if(!ofp)
		return;

	f_close(&ofp->file);

	sdcard_free_fd(fd);
}

int32_t sdcard_file_rem(fd_t fd)
{
	if (!SDCARD_FD_VALID(fd))
		return REM_INVALID;

	sdcard_open_file_t* ofp = sdcard_ofsp_from_fd(fd);

	if (ofp)
		return (((int32_t)ofp->file.fsize) - ofp->file.fptr);

	return REM_INVALID;
}

int32_t sdcard_file_read(fd_t fd, uint8_t* buf, int32_t len)
{
	if (!SDCARD_FD_VALID(fd) || !buf || !len)
		return RW_INVALID;

	sdcard_open_file_t* ofp = sdcard_ofsp_from_fd(fd);

	UINT rsiz;
	FRESULT res;
	if ((res = f_read(&ofp->file, buf, len, &rsiz)) == FR_OK)
	{
		return rsiz;
	}

	return RW_INVALID;
}

int32_t sdcard_file_write(fd_t fd, const uint8_t* buf, int32_t len)
{
	if (!SDCARD_FD_VALID(fd) || !buf || !len)
		return RW_INVALID;

	sdcard_open_file_t* ofp = sdcard_ofsp_from_fd(fd);

	UINT wsiz;
	FRESULT res;
	if ((res = f_write(&ofp->file, buf, len, &wsiz)) == FR_OK)
		return wsiz;

	return RW_INVALID;
}

int32_t sdcard_file_seek(fd_t fd, int32_t pos)
{
	if(!SDCARD_FD_VALID(fd))
		return SEEK_INVALID;

	sdcard_open_file_t* ofp = sdcard_ofsp_from_fd(fd);

	FRESULT res;
	if((res = f_lseek(&ofp->file, pos)) != FR_OK)
		return SEEK_INVALID;

	return pos;
}

uint32_t sdcard_file_ioctl(fd_t fd, uint32_t mask, void* arg)
{
	return IOCTL_INVALID;
}

const fsys_funmap_t sdcard_funmap =
{
		.listdir = sdcard_listdir,
		.open = sdcard_open,
		.unmount = sdcard_unmount,
		.unlink = sdcard_unlink,
		.rwdir = sdcard_rwdir,
		.chdir = sdcard_chdir,
		.mkdir = sdcard_mkdir
};

const fd_funmap_t sdcard_file_funmap =
{
		.close = sdcard_file_close,
		.ioctl = NULL,
		.read = sdcard_file_read,
		.seek = sdcard_file_seek,
		.write = sdcard_file_write,
		.rem = sdcard_file_rem
};
