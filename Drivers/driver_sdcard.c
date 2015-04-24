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

//const fsys_funmap_t sdcard_funmap =
//{
//		.listdir = sdcard_listdir,
//		.open = sdcard_open,
//		.touch = sdcard_touch,
//		.unmount = sdcard_unmount
//};
//
//const fd_funmap_t sdcard_file_funmap =
//{
//		.close = sdcard_file_close,
//		.ioctl = NULL,
//		.read = sdcard_file_read,
//		.seek = NULL,
//		.write = sdcard_file_write
//};

typedef struct
{
	FATFS fatfs;
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

	FILINFO file_info;
	DIR dir_info;

	f_opendir(&dir_info, (const TCHAR*)"/");

	do{
		f_readdir(&dir_info, &file_info);
		if((file_info.fattrib & AM_DIR) != AM_DIR)
		{
			Serial_puts(UART_DEBUG_MODULE, file_info.fname, 100);
			Serial_puts(UART_DEBUG_MODULE, "\r\n", 2);
		}
	} while(file_info.fname[0] != 0);
}
