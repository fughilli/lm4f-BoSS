/*
 * driver_hy28a.h
 *
 *  Created on: May 8, 2015
 *      Author: Kevin
 */

#ifndef DRIVER_HY28A_H_
#define DRIVER_HY28A_H_

#include "../file.h"
#include <stdint.h>

/*
 * Display modes: graphics/terminal
 *
 * functions for moving sections of the screen around
 * printing characters
 * maintain a character buffer
 */

#define HY28A_CMASK_RED 0xF800
#define HY28A_CMASK_GREEN 0x07E0
#define HY28A_CMASK_BLUE 0x001F

#define HY28A_CCOMP_TO_RED(_comp_) ((((_comp_) >> 3) << 11) & HY28A_CMASK_RED)
#define HY28A_CCOMP_TO_GREEN(_comp_) ((((_comp_) >> 2) << 5) & HY28A_CMASK_GREEN)
#define HY28A_CCOMP_TO_BLUE(_comp_) ((((_comp_) >> 3) << 0) & HY28A_CMASK_BLUE)

#define HY28A_CRGB_TO_COL(_R_,_G_,_B_) (HY28A_CCOMP_TO_RED((_R_)) | HY28A_CCOMP_TO_GREEN((_G_)) | HY28A_CCOMP_TO_BLUE((_B_)))

#define HY28A_COL_WHITE (HY28A_CRGB_TO_COL(255,255,255))
#define HY28A_COL_GRAY (HY28A_CRGB_TO_COL(128,128,128))
#define HY28A_COL_RED (HY28A_CRGB_TO_COL(255,0,0))
#define HY28A_COL_GREEN (HY28A_CRGB_TO_COL(0,255,0))
#define HY28A_COL_BLUE (HY28A_CRGB_TO_COL(0,0,255))
#define HY28A_COL_BLACK (HY28A_CRGB_TO_COL(0,0,0))

#define HY28A_MODE_CHARACTER (0x8000)
#define HY28A_MODE_GRAPHIC (0x4000)
#define HY28A_SET_WINDOW (0x2000)

typedef struct
{
	uint16_t x;
	uint16_t y;
	uint16_t w;
	uint16_t h;
} hy28a_window_t;

extern const fd_funmap_t hy28a_funmap;

#endif /* DRIVER_HY28A_H_ */
