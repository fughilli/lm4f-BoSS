/*
 * driver_hy28a.c
 *
 *  Created on: May 9, 2015
 *      Author: Kevin
 */

/*
 * driver_NHD0420D3Z_FL_GBW.c
 *
 *  Created on: Apr 2, 2015
 *      Author: Kevin
 */
#include "driver_hy28a.h"

#include "../fast_utils.h"
#include "SPI/SPI.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "Fonts/font5x7.h"

#define HY28A_WIDTH (40)
#define HY28A_HEIGHT (40)

#define HY28A_FONTW (6)
#define HY28A_FONTH (8)

#define HY28A_BACKSPACE (8)

uint8_t hy28a_buffer[HY28A_WIDTH * HY28A_HEIGHT];

uint8_t _cx = 0, _cy = 0;

bool consoleMode = false;

bool initialized = false;

void hy28a_putc(char c);

void hy28a_clear(bool clearbuffer)
{
	//TODO: clear screen

	if(clearbuffer)
		fast_memset(hy28a_buffer, ' ', HY28A_WIDTH*HY28A_HEIGHT);

	_cx = _cy = 0;
}

#define CS_HIGH() (GPIOPinWrite(GPIO_PORTA_BASE, 0x08, 0x08))
#define CS_LOW() (GPIOPinWrite(GPIO_PORTA_BASE, 0x08, 0x00))

#define RST_HIGH() (GPIOPinWrite(GPIO_PORTE_BASE, 0x01, 0x01))
#define RST_LOW() (GPIOPinWrite(GPIO_PORTE_BASE, 0x01, 0x00))

#define ILIGRAMMODE 0x03			// Define scan mode
#define SPI_START   0x70			// Start byte for SPI transfer
#define SPI_RD      0x01			// WR bit 1 within start
#define SPI_WR      0x00			// WR bit 0 within start
#define SPI_DATA    0x02			// RS bit 1 within start byte
#define SPI_INDEX   0x00			// RS bit 0 within start byte 0x00

#define _writeSPI(_data_) (SPI_transfer(0, (_data_)))

static void _writeCommand(uint8_t command8)
{
    CS_LOW();

    _writeSPI(SPI_START | SPI_WR | SPI_INDEX); // write : RS = 0, RW = 0
    _writeSPI(0x00); // write D8..D15
    _writeSPI(command8); // write D0..D7

    CS_HIGH();

}

static void _writeData16(uint16_t data16)
{
	CS_LOW();

    _writeSPI(SPI_START | SPI_WR | SPI_DATA); // write : RS = 1, RW = 0
    _writeSPI((data16 >> 8) & 0xFF); // write D8..D15
    _writeSPI(data16 & 0xFF); // write D0..D7

    CS_HIGH();

}

static void _writeRegister(uint8_t command8, uint16_t data16)
{
    _writeCommand(command8);
    _writeData16(data16);
}

static void _setOrientation(uint8_t orient)
{
	switch (orient)
	{
	case 0:
		_writeRegister(ILIGRAMMODE, 0x1000 + 0x30);
		break;
	case 1:
		_writeRegister(ILIGRAMMODE, 0x1000 + 0x28);
		break;
	case 2:
		_writeRegister(ILIGRAMMODE, 0x1000 + 0x00);
		break;
	case 3:
		_writeRegister(ILIGRAMMODE, 0x1000 + 0x18);
		break;
	}
}

static void _setWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    _writeRegister(0x20, x1);
	_writeRegister(0x21, y1);

	_writeRegister(0x50, x1);
	_writeRegister(0x52, y1);
	_writeRegister(0x51, x2);
	_writeRegister(0x53, y2);
	_writeCommand(0x22);
}

static void _setPoint(uint16_t x1, uint16_t y1, uint16_t color)
{
    _setWindow(x1, y1, x1, y1);
    _writeData16(color);
}

void hy28a_init()
{
	SPI_begin(0);
	SPI_setClockDivider(0, SPI_CLOCK_DIV2);
	SPI_setDataMode(0, SPI_MODE3);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, 0x08);
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, 0x01);

	RST_LOW();
	SysCtlDelay(1000);
	RST_HIGH();
	SysCtlDelay(1000);

	_writeRegister(0x00, 0x0000); // Start oscillation
	_writeRegister(0x01, 0x0100); // Driver Output Contral
	_writeRegister(0x02, 0x0700); // LCD Driver Waveform Contral
	_writeRegister(0x03, 0x1038); // Set the scan mode
	_writeRegister(0x04, 0x0000); // Scalling Contral
	_writeRegister(0x08, 0x0202); // Display Contral 2
	_writeRegister(0x09, 0x0000); // Display Contral 3
	_writeRegister(0x0a, 0x0000); // Frame Cycle Contal
	_writeRegister(0x0c, (1 << 0)); // Extern Display Interface Contral 1
	_writeRegister(0x0d, 0x0000); // Frame Maker Position
	_writeRegister(0x0f, 0x0000); // Extern Display Interface Contral 2
	SysCtlDelay(SysCtlClockGet() / 20 / 3);
	_writeRegister(0x07, 0x0101); // Display Contral
	SysCtlDelay(SysCtlClockGet() / 20 / 3);
	_writeRegister(0x10, (1 << 12) | (0 << 8) | (1 << 7) | (1 << 6) | (0 << 4)); // Power Control 1
	_writeRegister(0x11, 0x0007); // Power Control 2
	_writeRegister(0x12, (1 << 8) | (1 << 4) | (0 << 0)); // Power Control 3
	_writeRegister(0x13, 0x0b00); // Power Control 4
	_writeRegister(0x29, 0x0000); // Power Control 7
	_writeRegister(0x2b, (1 << 14) | (1 << 4));

	_writeRegister(0x50, 0); // Set X Start
	_writeRegister(0x51, 239);	 // Set X End
	_writeRegister(0x52, 0);	 // Set Y Start
	_writeRegister(0x53, 319);	 // Set Y End
	SysCtlDelay(SysCtlClockGet() / 20 / 3);

	_writeRegister(0x60, 0x2700); // Driver Output Control
	_writeRegister(0x61, 0x0001); // Driver Output Control
	_writeRegister(0x6a, 0x0000); // Vertical Srcoll Control

	_writeRegister(0x80, 0x0000); // Display Position? Partial Display 1
	_writeRegister(0x81, 0x0000); // RAM Address Start? Partial Display 1
	_writeRegister(0x82, 0x0000); // RAM Address End-Partial Display 1
	_writeRegister(0x83, 0x0000); // Displsy Position? Partial Display 2
	_writeRegister(0x84, 0x0000); // RAM Address Start? Partial Display 2
	_writeRegister(0x85, 0x0000); // RAM Address End? Partial Display 2

	_writeRegister(0x90, (0 << 7) | (16 << 0)); // Frame Cycle Contral
	_writeRegister(0x92, 0x0000); // Panel Interface Contral 2
	_writeRegister(0x93, 0x0001); // Panel Interface Contral 3
	_writeRegister(0x95, 0x0110); // Frame Cycle Contral
	_writeRegister(0x97, (0 << 8));
	_writeRegister(0x98, 0x0000); // Frame Cycle Contral
	_writeRegister(0x07, 0x0133);
	SysCtlDelay(SysCtlClockGet() / 10 / 3);

	_setOrientation(0);

	hy28a_clear(true);
}

void hy28a_shiftUp()
{
	fast_memmove(hy28a_buffer, &hy28a_buffer[HY28A_WIDTH], HY28A_WIDTH*(HY28A_HEIGHT-1));
	fast_memset(&hy28a_buffer[HY28A_WIDTH*(HY28A_HEIGHT-1)], ' ', HY28A_WIDTH);

	uint8_t old_cx = _cx;

	hy28a_clear(false);

	int i;
	for(i = 0; i < (HY28A_WIDTH*(HY28A_HEIGHT-1)); i++)
	{
		hy28a_putc(hy28a_buffer[i]);
	}

	_cx = old_cx;
	_cy = HY28A_HEIGHT-1;
}

void hy28a_back()
{
	if(_cx)
		_cx--;
	else
	{
		if(_cy)
		{
			_cy--;
		}
		else
		{
			_cy = HY28A_HEIGHT-1;
		}
		_cx = HY28A_WIDTH-1;
	}
}

typedef struct
{
	const uint8_t* data;
	uint16_t width;
	uint16_t height;
	uint16_t bwidth;
} Bitmap_t;

typedef struct
{
	Bitmap_t bmp;
	uint8_t width;
	uint8_t height;
	uint8_t stride;
	uint8_t hstride;
	uint8_t vstride;
} Font_t;

static const Font_t hy28a_font =
{
		.bmp = {
				.bwidth = 160,
				.width = 1280,
				.height = 7,
				.data = font5x7
		},
		.width = 5,
		.height = 7,
		.stride = 5,
		.hstride = 6,
		.vstride = 8
};

static uint8_t getfontpixel(const Font_t* font, char c, uint8_t x, uint8_t y)
{
	uint32_t offset = font->bmp.bwidth;
	offset *= y;
	uint32_t lineoffset = ((c * font->stride) + x);
	offset += (lineoffset >> 3);
	lineoffset = 7 - (lineoffset & 7);
	return ((font->bmp.data[offset] >> lineoffset) & 1);
}

static void hy28a_writechar(char c)
{
	int destx = _cx * HY28A_FONTW;
	int desty = _cy * HY28A_FONTH;

	int i, j;

	bool space = (c == ' ');

	for(i = 0; i < HY28A_FONTH; i++)
	{
		for(j = 0; j < HY28A_FONTW; j++)
		{
			if(space)
			{
				_setPoint(destx + j, desty + i, HY28A_COL_BLACK);
				continue;
			}

			if(getfontpixel(&hy28a_font, c, j, i))
			{
				_setPoint(destx + j, desty + i, HY28A_COL_WHITE);
			}
		}
	}
}

void hy28a_putc(char c)
{
	int i;
	switch(c)
	{
	case '\n':
		if (consoleMode)
		{
			if (_cy < HY28A_HEIGHT - 1)
			{
				_cy++;
			}
			else
				hy28a_shiftUp();
		}
		else
		{
			_cy++;
			if (_cy == HY28A_HEIGHT)
			{
				_cy = 0;
			}
		}
		break;
	case '\r':
		_cx = 0;
		break;
	case '\t':
		while(i < 4)
		{
			hy28a_putc(' ');
			i++;
		}
		break;
	case HY28A_BACKSPACE:
		hy28a_back();
		hy28a_putc(' ');
		hy28a_back();
		break;
	default:
		hy28a_buffer[_cx + (_cy * HY28A_WIDTH)] = c;
		hy28a_writechar(c);
		_cx++;
		if(_cx == HY28A_WIDTH)
		{
			_cx = 0;
			if(consoleMode)
			{
				if(_cy < HY28A_HEIGHT-1)
					_cy++;
				else
					hy28a_shiftUp();

			}
			else
			{
				_cy++;
				if(_cy == HY28A_HEIGHT)
					_cy = 0;
			}
		}
		break;
	}
}

int32_t hy28a_write(fd_t fd, const uint8_t* buf, int32_t len)
{
	if(!initialized)
	{
		initialized = true;
		hy28a_init();
	}

	int32_t i;
	for(i = 0; i < len; i++)
	{
		hy28a_putc(buf[i]);
	}

	return len;
}

uint32_t hy28a_ioctl(fd_t fd, uint32_t flags, void* arg)
{
	return IOCTL_INVALID;
}

const fd_funmap_t hy28a_funmap =
{
		.close = NULL,
		.read = NULL,
		.write = hy28a_write,
		.seek = NULL,
		.ioctl = NULL

};



