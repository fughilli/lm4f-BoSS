/*
 * driver_debuguart.c
 *
 *  Created on: Apr 30, 2015
 *      Author: Kevin
 */

#include "driver_debuguart.h"
#include "../debug_serial.h"
#include "../fast_utils.h"

void debuguart_init()
{
	Serial_init(UART_DEBUG_MODULE, 115200);
}

void debuguart_close(fd_t fd)
{
	;
}

int32_t debuguart_read(fd_t fd, uint8_t* buf, int32_t len)
{
	int32_t i = 0;
	while(i < len)// && Serial_avail(UART_DEBUG_MODULE))
	{
		buf[i] = Serial_getc(UART_DEBUG_MODULE);
		if(buf[i] == ((uint8_t)-1))
			continue;
		i++;
	}
	return i;
}

int32_t debuguart_write(fd_t fd, const uint8_t* buf, int32_t len)
{
	Serial_writebuf(UART_DEBUG_MODULE, buf, len);
	return len;
}

int32_t debuguart_seek(fd_t fd, int32_t pos)
{
	return SEEK_INVALID;
}

uint32_t debuguart_ioctl(fd_t fd, uint32_t mask, void* arg)
{
	return IOCTL_INVALID;
}

const fd_funmap_t debuguart_funmap =
{
	.close = debuguart_close,
	.ioctl = NULL,
	.seek = NULL,
	.read = debuguart_read,
	.write = debuguart_write
};
