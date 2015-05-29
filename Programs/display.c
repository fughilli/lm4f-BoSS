/*
 * display.c
 *
 *  Created on: May 10, 2015
 *      Author: Kevin
 */

#include "display.h"
#include "../Drivers/driver_hy28a.h"

int display_main(char* argv[], int argc)
{
	if(argc != 2)
	{
		s_puts("Invalid arguments!\r\n");
		return -1;
	}

	fd_t imgfd = sys_open(argv[1], FMODE_R, 0);
	uint8_t imgbuf[48];
	int32_t readidx;

	if (imgfd != FD_INVALID)
	{
		// Read the header (2b width, 2b height)
		readidx = sys_read(imgfd, imgbuf, 4);
		if (readidx != 4)
			goto _bootsplash_end;

		uint16_t imgw = imgbuf[0];
		imgw <<= 8;
		imgw |= imgbuf[1];
		uint16_t imgh = imgbuf[2];
		imgh <<= 8;
		imgh |= imgbuf[3];

		hy28a_window_t windowargs =
		{ .x = 0, .y = 0, .w = imgw, .h = imgh };

		sys_ioctl(STDOUT, HY28A_MODE_GRAPHIC, NULL);

		if(sys_ioctl(STDOUT, HY28A_SET_WINDOW, &windowargs) == IOCTL_INVALID)
		{
			s_puts("Image is too big!\r\n");
			goto _bootsplash_end;
		}

		while (sys_rem(imgfd) && (readidx = sys_read(imgfd, imgbuf, 48)) != RW_INVALID)
		{
			if (readidx == 0)
				break;

			sys_write(STDOUT, imgbuf, readidx);
		}

		_bootsplash_end: sys_ioctl(STDOUT, HY28A_MODE_CHARACTER, NULL);

		sys_close(imgfd);
	}
	else
	{
		s_puts("Failed to open file!\r\n");
		return -1;
	}

	return 0;
}
