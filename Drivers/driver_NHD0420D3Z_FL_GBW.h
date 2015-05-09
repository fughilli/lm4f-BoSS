/*
 * driver_NHD0420D3Z_FL_GBW.h
 *
 *  Created on: Apr 2, 2015
 *      Author: Kevin
 */

#ifndef DRIVER_NHD0420D3Z_FL_GBW_H_
#define DRIVER_NHD0420D3Z_FL_GBW_H_

#include "../file.h"
#include <stdbool.h>
#include <stdint.h>

extern const fd_funmap_t nhd_funmap;

typedef union
{
	uint32_t raw;
	struct
	{
		uint8_t cursorx;
		uint8_t cursory;

		uint16_t control;
	} s;
} nhd_ioctl_flags_t;

// Control mask flags
#define NHD_CONTMASK_UNDERLINEON (0x0002)
#define NHD_CONTMASK_UNDERLINEOFF (0x0004)

#define NHD_CONTMASK_POWERON (0x0008)
#define NHD_CONTMASK_POWEROFF (0x0010)

#define NHD_CONTMASK_BLINKON (0x0020)
#define NHD_CONTMASK_BLINKOFF (0x0040)

#define NHD_CONTMASK_CONSOLEMODEON (0x0080)
#define NHD_CONTMASK_CONSOLEMODEOFF (0x0100)

#define NHD_CONTMASK_CLEAR (0x2000)
// Write the control info to the display
#define NHD_CONTMASK_WCONT (0x8000)
// Write the cursor position to the display
#define NHD_CONTMASK_WPOS (0x4000)

uint32_t nhd_makeflags(uint8_t cx, uint8_t cy, bool wpos, uint16_t ctl, bool wctl);

#endif /* DRIVER_NHD0420D3Z_FL_GBW_H_ */
