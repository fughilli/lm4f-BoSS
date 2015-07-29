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

#ifndef DRIVER_HY28A_C_
#define DRIVER_HY28A_C_

#include "driver_hy28a.h"

#include "../fast_utils.h"
#include "SPI/SPI.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "Fonts/font5x7.h"
#include "Fonts/font9x15.h"

//#define HY28A_PIX_WIDTH (240)
//#define HY28A_PIX_HEIGHT (320)

//#define HY28A_WIDTH (40)
//#define HY28A_HEIGHT (40)

//#define HY28A_FONTW (6)
//#define HY28A_FONTH (8)

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
	uint8_t (*map)(uint8_t);
} Font_t;

typedef struct
{
    uint16_t y1;
    uint16_t y2;
    uint16_t off;
} pimg_t;

pimg_t pimg1, pimg2, *pimgp1, *pimgp2;

static uint16_t
hy28a_buf_w, hy28a_buf_h,
hy28a_pix_w, hy28a_pix_h;

#define HY28A_BACKSPACE (8)
#define HY28A_FORM_FEED (12)

#define HY28A_BELL_CHAR (7)

uint8_t hy28a_font_5x7_map(uint8_t c)
{
	return c;
}

static const Font_t hy28a_font_5x7 =
{
		.bmp = {
				.bwidth = 160,
				.width = 1280,
				.height = 7,
				.data = font5x7
		},
		.map = hy28a_font_5x7_map,
		.width = 5,
		.height = 7,
		.stride = 5,
		.hstride = 6,
		.vstride = 8
};

uint8_t hy28a_font_9x15_map(uint8_t c)
{
	if(c < 128 && c >= ' ')
		return (c - ' ');
	return 0;
}

static const Font_t hy28a_font_9x15 =
{
		.bmp = {
				.bwidth = 107,
				.width = 855,
				.height = 15,
				.data = font9x15
		},
		.map = hy28a_font_9x15_map,
		.width = 9,
		.height = 15,
		.stride = 9,
		.hstride = 10,
		.vstride = 16
};

static uint8_t* hy28a_buffer;

static uint8_t _cx = 0, _cy = 0;

static bool consoleMode = true;

static bool initialized = false;

static Font_t hy28a_font;

typedef enum
{
	PORTRAIT,
	LANDSCAPE,
	PORTRAIT_INV,
	LANDSCAPE_INV
} hy28a_disp_orient_t;

hy28a_disp_orient_t hy28a_orient;

typedef enum
{
	PARTIAL_DISPLAY_1 = 0,
	PARTIAL_DISPLAY_2 = 1
} pdisp_num_t;

//static uint16_t _gcx = 0, _gcy = 0;

void hy28a_putc(char c);
void hy28a_fast_fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

void hy28a_clear(bool clearbuffer)
{
	hy28a_fast_fill(0, 0, hy28a_pix_w - 1, hy28a_pix_h - 1, 0x0000);

	pimg1.y1 = 0;
	pimg1.y2 = hy28a_font.vstride;
	pimg1.off = 0;

	pimg2.y1 = 0;
	pimg2.y2 = hy28a_buf_h*hy28a_font.vstride;
	pimg2.off = 0;

	pimgp1 = &pimg1;
	pimgp2 = &pimg2;

	hy28a_set_partial_display(PARTIAL_DISPLAY_1,
			pimg1.y1,
			pimg1.y2,
			pimg1.off);
	hy28a_set_partial_display(PARTIAL_DISPLAY_2,
			pimg2.y1,
			pimg2.y2,
			pimg2.off);

	if(clearbuffer)
		fast_memset((void*)hy28a_buffer, 0, hy28a_buf_w*hy28a_buf_h);

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

//#define _writeSPI(_data_) (SPI_transfer(0, (_data_)))
#define _writeSPI(_data_) while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_TNF)); HWREG(SSI0_BASE + SSI_O_DR) = (_data_); while(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_BSY)

static void hy28a_write_command(uint8_t command8)
{
    CS_LOW();

    _writeSPI(SPI_START | SPI_WR | SPI_INDEX); // write : RS = 0, RW = 0
    _writeSPI(0x00); // write D8..D15
    _writeSPI(command8); // write D0..D7

    CS_HIGH();

}

static void hy28a_write_data16(uint16_t data16)
{
	CS_LOW();

    _writeSPI(SPI_START | SPI_WR | SPI_DATA); // write : RS = 1, RW = 0
    _writeSPI((data16 >> 8) & 0xFF); // write D8..D15
    _writeSPI(data16 & 0xFF); // write D0..D7

    CS_HIGH();

}

static void hy28a_write_register(uint8_t command8, uint16_t data16)
{
    hy28a_write_command(command8);
    hy28a_write_data16(data16);
}

static void hy28a_refresh_buffer()
{
	hy28a_buf_w = hy28a_pix_w / hy28a_font.hstride;
	hy28a_buf_h = hy28a_pix_h / hy28a_font.vstride;

	fast_free(hy28a_buffer);
	hy28a_buffer = (uint8_t*) fast_alloc(hy28a_buf_w * hy28a_buf_h);
}

static void hy28a_set_orientation(hy28a_disp_orient_t orient)
{
	switch (orient)
	{
	case PORTRAIT:
		hy28a_write_register(ILIGRAMMODE, 0x1000 + 0x30);
		hy28a_pix_h = 320;
		hy28a_pix_w = 240;
		break;
	case LANDSCAPE:
		hy28a_write_register(ILIGRAMMODE, 0x1000 + 0x28);
		hy28a_pix_h = 240;
		hy28a_pix_w = 320;
		break;
	case PORTRAIT_INV:
		hy28a_write_register(ILIGRAMMODE, 0x1000 + 0x00);
		hy28a_pix_h = 320;
		hy28a_pix_w = 240;
		break;
	case LANDSCAPE_INV:
		hy28a_write_register(ILIGRAMMODE, 0x1000 + 0x18);
		hy28a_pix_h = 240;
		hy28a_pix_w = 320;
		break;
	}

	if(hy28a_orient != orient)
	{
		hy28a_refresh_buffer();
	}
	hy28a_orient = orient;
}

static void hy28a_rotate_coord(uint16_t* px, uint16_t* py)
{
	switch (hy28a_orient)
	{
	case PORTRAIT:
		break;
	case LANDSCAPE:
		(*py) = 240 - (*py) - 1;
		fast_swap16(px, py);
		break;
	case PORTRAIT_INV:
		(*px) = 240 - (*px) - 1;
		(*py) = 320 - (*py) - 1;
		break;
	case LANDSCAPE_INV:
		(*px) = 320 - (*px) - 1;
		fast_swap16(px, py);
		break;
	}
}

static void hy28a_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	hy28a_rotate_coord(&x1, &y1);
	hy28a_rotate_coord(&x2, &y2);

	hy28a_write_register(0x20, x1);
	hy28a_write_register(0x21, y1);

	if(x1 > x2) fast_swap16(&x1, &x2);
	if(y1 > y2) fast_swap16(&y1, &y2);

	hy28a_write_register(0x50, x1);
	hy28a_write_register(0x52, y1);
	hy28a_write_register(0x51, x2);
	hy28a_write_register(0x53, y2);
	hy28a_write_command(0x22);
}

void hy28a_set_partial_display(pdisp_num_t pdn, uint16_t y1, uint16_t y2, uint16_t yoff)
{
	uint8_t register_offset = 3 * pdn;

	hy28a_write_register(0x80 + register_offset, yoff);
	hy28a_write_register(0x81 + register_offset, y1);
	hy28a_write_register(0x82 + register_offset, y2);
}

void hy28a_fast_fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    hy28a_set_window(x1, y1, x2, y2);
    hy28a_write_command(0x0022);

    CS_LOW();

    _writeSPI(SPI_START | SPI_WR | SPI_DATA); // write : RS = 1, RW = 0

    uint8_t highColor = (color >> 8) & 0xFF;
    uint8_t lowColor  = (color) & 0xFF;
    uint32_t t;

    for (t = (uint32_t)(y2-y1+1)*(x2-x1+1); t>0; t--) {
        _writeSPI(highColor); // write D8..D15
        _writeSPI(lowColor); // write D0..D7
    }

    CS_HIGH();
}

static void hy28a_load_font(const Font_t* font)
{
	fast_memcpy(&hy28a_font, font, sizeof(Font_t));
	hy28a_refresh_buffer();
}

typedef struct
{
	uint8_t addr;
	uint16_t val;
} hy28a_register_val_t;


hy28a_register_val_t initregvals[] __attribute__((section("FLASH"))) =
{
		{0x00, 0x0000},
		{0x01, 0x0100},
		{0x02, 0x0700},
		{0x03, 0x1038},

};

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

	hy28a_write_register(0x00, 0x0000); // Start oscillation
	hy28a_write_register(0x01, 0x0100); // Driver Output Contral
	hy28a_write_register(0x02, 0x0700); // LCD Driver Waveform Contral
	hy28a_write_register(0x03, 0x1038); // Set the scan mode
	hy28a_write_register(0x04, 0x0000); // Scalling Contral
	hy28a_write_register(0x08, 0x0202); // Display Contral 2
	hy28a_write_register(0x09, 0x0000); // Display Contral 3
	hy28a_write_register(0x0a, 0x0000); // Frame Cycle Contal
	hy28a_write_register(0x0c, 0x0001); // 8.2.12: 16-bit RGB/One transfer per pixel
	hy28a_write_register(0x0d, 0x0000); // 8.2.13: No frame marker
	hy28a_write_register(0x0f, 0x0000); // 8.2.14

	SysCtlDelay(SysCtlClockGet() / 20 / 3);

	hy28a_write_register(0x07, 0x0101); // Display Contral

	SysCtlDelay(SysCtlClockGet() / 20 / 3);
	hy28a_write_register(0x10, (1 << 12) | (0 << 8) | (1 << 7) | (1 << 6) | (0 << 4)); // Power Control 1
	hy28a_write_register(0x11, 0x0007); // Power Control 2
	hy28a_write_register(0x12, (1 << 8) | (1 << 4) | (0 << 0)); // Power Control 3
	hy28a_write_register(0x13, 0x0b00); // Power Control 4
	hy28a_write_register(0x29, 0x0000); // Power Control 7
	hy28a_write_register(0x2b, (1 << 14) | (1 << 4));

	hy28a_write_register(0x50, 0); // Set X Start
	hy28a_write_register(0x51, 239);	 // Set X End
	hy28a_write_register(0x52, 0);	 // Set Y Start
	hy28a_write_register(0x53, 319);	 // Set Y End
	SysCtlDelay(SysCtlClockGet() / 20 / 3);

	hy28a_write_register(0x60, 0x2700); // Driver Output Control
	hy28a_write_register(0x61, 0x0001); // Driver Output Control
	hy28a_write_register(0x6a, 0x0000); // Vertical Srcoll Control

	hy28a_write_register(0x80, 0x0000); // Display Position? Partial Display 1
	hy28a_write_register(0x81, 0x0000); // RAM Address Start? Partial Display 1
	hy28a_write_register(0x82, 0x0000); // RAM Address End-Partial Display 1
	hy28a_write_register(0x83, 0x0000); // Displsy Position? Partial Display 2
	hy28a_write_register(0x84, 0x0000); // RAM Address Start? Partial Display 2
	hy28a_write_register(0x85, 0x0000); // RAM Address End? Partial Display 2

	hy28a_write_register(0x90, (0 << 7) | (16 << 0)); // Frame Cycle Contral
	hy28a_write_register(0x92, 0x0000); // Panel Interface Contral 2
	hy28a_write_register(0x93, 0x0001); // Panel Interface Contral 3
	hy28a_write_register(0x95, 0x0110); // Frame Cycle Contral
	hy28a_write_register(0x97, (0 << 8));
	hy28a_write_register(0x98, 0x0000); // Frame Cycle Contral
	//hy28a_write_register(0x07, 0x0133);
	hy28a_write_register(0x07, 0x3033);
	SysCtlDelay(SysCtlClockGet() / 10 / 3);

	hy28a_pix_w = 240;
	hy28a_pix_h = 320;

	hy28a_buf_w = hy28a_pix_w / hy28a_font.hstride;
	hy28a_buf_h = hy28a_pix_h / hy28a_font.vstride;

	hy28a_buffer = (uint8_t*) fast_alloc(hy28a_buf_w * hy28a_buf_h);

	hy28a_orient = PORTRAIT;

	hy28a_load_font(&hy28a_font_9x15);

	hy28a_set_orientation(PORTRAIT);

	hy28a_clear(true);

	consoleMode = true;
}

void hy28a_shiftUp()
{
	//fast_memmove((void*)hy28a_buffer, (void*)&hy28a_buffer[hy28a_buf_w], hy28a_buf_w*(hy28a_buf_h-1));
	//fast_memset((void*)&hy28a_buffer[hy28a_buf_w*(hy28a_buf_h-1)], 0, hy28a_buf_w);

	if(pimgp1->y2 < hy28a_buf_h*hy28a_font.vstride)
	{
		pimgp1->y2 += hy28a_font.vstride;

		pimgp2->y1 += hy28a_font.vstride;
		pimgp2->off += hy28a_font.vstride;

		if (pimgp1->y2 == hy28a_buf_h * hy28a_font.vstride)
		{
			pimgp2->y1 = 0;
			pimgp2->y2 = 0;
			pimgp2->off = hy28a_buf_h * hy28a_font.vstride;
		}
	}
	else
	{
		if (pimgp1->y1 < hy28a_buf_h * hy28a_font.vstride)
		{
			pimgp1->y1 += hy28a_font.vstride;
			pimgp2->off -= hy28a_font.vstride;
			pimgp2->y2 += hy28a_font.vstride;

			if (pimgp1->y1 == hy28a_buf_h * hy28a_font.vstride)
			{
				pimgp1->off = hy28a_buf_h * hy28a_font.vstride;
				pimgp1->y1 = 0;
				pimgp1->y2 = 0;

				pimg_t* temp = pimgp1;
				pimgp1 = pimgp2;
				pimgp2 = temp;
			}
		}
	}

	hy28a_set_partial_display(PARTIAL_DISPLAY_1,
			pimg1.y1,
			pimg1.y2,
			pimg1.off);
	hy28a_set_partial_display(PARTIAL_DISPLAY_2,
			pimg2.y1,
			pimg2.y2,
			pimg2.off);

	_cy++;
	if(_cy == hy28a_buf_h)
	{
		_cy = 0;
	}

	int desty = _cy * hy28a_font.vstride;

	hy28a_set_window(0, desty, hy28a_pix_w, desty + hy28a_font.vstride);

	CS_LOW();

	_writeSPI(SPI_START | SPI_WR | SPI_DATA);

	int i, j;

	for (i = 0; i < hy28a_font.vstride; i++)
	{
		for (j = 0; j < hy28a_pix_w; j++)
		{
			_writeSPI(HY28A_COL_BLACK >> 8);
			_writeSPI(HY28A_COL_BLACK & 0xFF);
		}
	}

	CS_HIGH();

//	uint8_t old_cx = _cx;
//
//	hy28a_clear(false);
//
//	int i;
//	for(i = 0; i < (hy28a_buf_w*(hy28a_buf_h-1)); i++)
//	{
//		hy28a_putc(hy28a_buffer[i]);
//	}
//
//	_cx = old_cx;
//	_cy = hy28a_buf_h-1;
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
			_cy = hy28a_buf_h-1;
		}
		_cx = hy28a_buf_w-1;
	}
}

static uint8_t getfontpixel(const Font_t* font, char c, uint8_t x, uint8_t y)
{
	c = font->map(c);
	uint32_t offset = font->bmp.bwidth;
	offset *= y;
	uint32_t lineoffset = ((c * font->stride) + x);
	offset += (lineoffset >> 3);
	lineoffset = 7 - (lineoffset & 7);
	return ((font->bmp.data[offset] >> lineoffset) & 1);
}

static void hy28a_writechar(char c)
{
	int destx = _cx * hy28a_font.hstride;
	int desty = _cy * hy28a_font.vstride;

	int i, j;

	bool space = (c == ' ');

	hy28a_set_window(destx, desty, destx + hy28a_font.width - 1, desty + hy28a_font.height - 1);

	CS_LOW();

	_writeSPI(SPI_START | SPI_WR | SPI_DATA);

	for (i = 0; i < hy28a_font.height; i++)
	{
		for (j = 0; j < hy28a_font.width; j++)
		{
			if (space)
			{
				_writeSPI(HY28A_COL_BLACK >> 8);
				_writeSPI(HY28A_COL_BLACK & 0xFF);
				continue;
			}

			if (!getfontpixel(&hy28a_font, c, j, i))
			{
				_writeSPI(HY28A_COL_WHITE >> 8);
				_writeSPI(HY28A_COL_WHITE & 0xFF);
			}
			else
			{
				_writeSPI(HY28A_COL_BLACK >> 8);
				_writeSPI(HY28A_COL_BLACK & 0xFF);
			}
		}
	}

	CS_HIGH();
}

void hy28a_putc(char c)
{
	int i;
	switch(c)
	{
	case '\n':
//		if (consoleMode)
//		{
//			if (_cy < hy28a_buf_h - 1)
//			{
//				_cy++;
//			}
//			else
//				hy28a_shiftUp();
//		}
//		else
//		{
//			_cy++;
//			if (_cy == hy28a_buf_h)
//			{
//				_cy = 0;
//			}
//		}
		hy28a_shiftUp();
		break;
	case '\r':
		_cx = 0;
		break;
	case HY28A_FORM_FEED:
		hy28a_clear(true);
		break;
	case '\t':
		i = 0;
		while(i < 4)
		{
			hy28a_putc(' ');
			i++;
		}
		break;
	case HY28A_BELL_CHAR:
		break;
	case HY28A_BACKSPACE:
		hy28a_back();
		break;
	default:
		hy28a_buffer[_cx + (_cy * hy28a_buf_w)] = c;
		hy28a_writechar(c);
		_cx++;
		if(_cx == hy28a_buf_w)
		{
			_cx = 0;
//			if(consoleMode)
//			{
//				if(_cy < hy28a_buf_h-1)
//					_cy++;
//				else
//					hy28a_shiftUp();
//
//			}
//			else
//			{
//				_cy++;
//				if(_cy == hy28a_buf_h)
//					_cy = 0;
//			}
			hy28a_shiftUp();
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

	if(consoleMode)
	{

	int32_t i;
	for(i = 0; i < len; i++)
	{
		hy28a_putc(buf[i]);
	}

	}
	else
	{
		if(len % 3)
			return RW_INVALID;

		int32_t i;

		for(i = 0; i < len; i += 3)
		{
			hy28a_write_data16(HY28A_CRGB_TO_COL(buf[i], buf[i+1], buf[i+2]));
		}
	}

	return len;
}

bool hy28a_bx(uint16_t x, uint16_t y)
{
	return (x < hy28a_pix_w && y < hy28a_pix_h);
}

uint32_t hy28a_ioctl(fd_t fd, uint32_t flags, void* arg)
{
	if (!initialized)
	{
		initialized = true;
		hy28a_init();
	}

	if((flags & (HY28A_MODE_CHARACTER | HY28A_MODE_GRAPHIC)) == HY28A_MODE_CHARACTER)
	{
		consoleMode = true;
		return HY28A_MODE_CHARACTER;
	} else if((flags & (HY28A_MODE_CHARACTER | HY28A_MODE_GRAPHIC)) == HY28A_MODE_GRAPHIC)
	{
		consoleMode = false;
		return HY28A_MODE_GRAPHIC;
	} else if(flags & HY28A_SET_WINDOW)
	{
		if(!arg)
			return IOCTL_INVALID;

		hy28a_window_t* win = (hy28a_window_t*)arg;

		if(!hy28a_bx(win->x+win->w-1, win->y+win->h-1))
		{
			return IOCTL_INVALID;
		}

		hy28a_set_window(win->x, win->y, win->x + win->w - 1, win->y + win->h - 1);

		//fast_memcpy(&curwindow, win, sizeof(hy28a_window_t));

		//_gcx = win->x;
		//_gcy = win->y;

		return HY28A_SET_WINDOW;
	}

	return IOCTL_INVALID;
}

const fd_funmap_t hy28a_funmap =
{
		.close = NULL,
		.read = NULL,
		.write = hy28a_write,
		.seek = NULL,
		.ioctl = hy28a_ioctl

};

#endif

