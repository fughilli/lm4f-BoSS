/*
 * driver_NHD0420D3Z_FL_GBW.c
 *
 *  Created on: Apr 2, 2015
 *      Author: Kevin
 */

#ifndef DRIVER_NHD0420D3Z_FL_GBW_C_
#define DRIVER_NHD0420D3Z_FL_GBW_C_

#include "driver_NHD0420D3Z_FL_GBW.h"

#include "../fast_utils.h"
#include "../debug_serial.h"

#define NHD_UART_MODULE (UART7_MODULE)

#define NHD_WIDTH (20)
#define NHD_HEIGHT (4)

uint8_t _cx = 0, _cy = 0;

void nhd_writepos(uint8_t cx, uint8_t cy)
{
	const uint8_t line_offsets[] =
	{ 0x00, 0x40, 0x14, 0x54 };
	uint8_t cursorControlBuf[3] =
	{ 0xFE, 0x45, 0x00 };
	cursorControlBuf[2] = line_offsets[cy] + cx;

	_cx = cx;
	_cy = cy;

	Serial_writebuf(NHD_UART_MODULE, cursorControlBuf, 3);
}

void nhd_clear()
{
	const uint8_t nhd_clear_buf[] = {0xFE, 0x51};
	Serial_writebuf(NHD_UART_MODULE, nhd_clear_buf, 2);

	_cx = _cy = 0;
}

int32_t nhd_write(fd_t fd, const uint8_t* buf, int32_t len)
{
	int32_t firstlen = len;
	while(len >= (NHD_WIDTH - _cx))
	{
		Serial_writebuf(NHD_UART_MODULE, &buf[firstlen - len], (uint32_t)(NHD_WIDTH - _cx));

		len -= (NHD_WIDTH - _cx);

		_cx = 0;
		_cy++;
		if(_cy == NHD_HEIGHT)
			_cy = 0;

		nhd_writepos(_cx, _cy);
	}

	if (len)
	{
		Serial_writebuf(NHD_UART_MODULE, &buf[firstlen - len], (uint32_t) len);
		_cx += len;
	}

	return len;
}

uint32_t nhd_ioctl(fd_t fd, uint32_t flags, void* arg)
{
	uint8_t cursorControlBuf[3] =
	{ 0xFE, 0x00, 0x00 };
	nhd_ioctl_flags_t sflags;
	sflags.raw = flags;

	uint16_t cflags = sflags.s.control;

	if (!(cflags & NHD_CONTMASK_WCONT) && !(cflags & NHD_CONTMASK_WPOS))
		return IOCTL_INVALID;

	uint32_t ret = 0;

	if (cflags & NHD_CONTMASK_WCONT)
	{
		if (cflags & NHD_CONTMASK_CLEAR)
		{
			nhd_clear();
			ret |= NHD_CONTMASK_CLEAR;
		}

		// Determine on/off flags; exclusive (both will not change anything)
		// Blinking Cursor
		if ((cflags & NHD_CONTMASK_BLINKON) == NHD_CONTMASK_BLINKON)
		{
			cursorControlBuf[1] = 0x4B;
			ret |= NHD_CONTMASK_BLINKON;
		}
		if ((cflags & NHD_CONTMASK_BLINKOFF) == NHD_CONTMASK_BLINKOFF)
		{
			cursorControlBuf[1] = 0x4C;
			ret |= NHD_CONTMASK_BLINKOFF;
		}

		if (cursorControlBuf[1])
		{
			Serial_writebuf(NHD_UART_MODULE, cursorControlBuf, 2);
		}
		cursorControlBuf[1] = 0;

		// Underline
		if ((cflags & NHD_CONTMASK_UNDERLINEON) == NHD_CONTMASK_UNDERLINEON)
		{
			cursorControlBuf[1] = 0x47;
			ret |= NHD_CONTMASK_UNDERLINEON;
		}
		if ((cflags & NHD_CONTMASK_UNDERLINEOFF) == NHD_CONTMASK_UNDERLINEOFF)
		{
			cursorControlBuf[1] = 0x48;
			ret |= NHD_CONTMASK_UNDERLINEOFF;
		}

		if (cursorControlBuf[1])
		{
			Serial_writebuf(NHD_UART_MODULE, cursorControlBuf, 2);
		}
		cursorControlBuf[1] = 0;

		// Power
		if ((cflags & NHD_CONTMASK_POWERON) == NHD_CONTMASK_POWERON)
		{
			cursorControlBuf[1] = 0x41;
			ret |= NHD_CONTMASK_POWERON;
		}
		if ((cflags & NHD_CONTMASK_POWEROFF) == NHD_CONTMASK_POWEROFF)
		{
			cursorControlBuf[1] = 0x42;
			ret |= NHD_CONTMASK_POWEROFF;
		}

		if (cursorControlBuf[1])
		{
			Serial_writebuf(NHD_UART_MODULE, cursorControlBuf, 2);
		}
		cursorControlBuf[1] = 0;

		if (ret)
			ret |= NHD_CONTMASK_WCONT;
	}

	if (cflags & NHD_CONTMASK_WPOS)
	{
		if (sflags.s.cursorx < NHD_WIDTH && sflags.s.cursory < NHD_HEIGHT)
		{
			nhd_writepos(sflags.s.cursorx, sflags.s.cursory);

			if (ret)
			{
				ret |= NHD_CONTMASK_WPOS;
				return ret;
			}

			return NHD_CONTMASK_WPOS;
		}
	}

	if (ret)
		return ret;

	return IOCTL_INVALID;
}

uint32_t nhd_makeflags(uint8_t cx, uint8_t cy, bool wpos, uint16_t ctl, bool wctl)
{
	nhd_ioctl_flags_t sflags;

	ctl |= ((wpos?NHD_CONTMASK_WPOS:0) | (wctl?NHD_CONTMASK_WCONT:0));
	sflags.s.control = ctl;
	sflags.s.cursorx = cx;
	sflags.s.cursory = cy;

	return sflags.raw;
}

const fd_funmap_t nhd_funmap =
{
		.close = NULL,
		.read = NULL,
		.write = nhd_write,
		.seek = NULL,
		.ioctl = nhd_ioctl

};

#endif /* DRIVER_NHD0420D3Z_FL_GBW_C_ */
