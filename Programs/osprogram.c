/*
 * osprogram.c
 *
 *  Created on: May 3, 2015
 *      Author: Kevin
 */

#include "osprogram.h"

void s_puts(const char * str)
{
	sys_write(STDOUT, (uint8_t*)str, fast_strlen(str));
}

void s_putc(char c)
{
	sys_write(STDOUT, (uint8_t*)&c, 1);
}
