/*
 * clear.c
 *
 *  Created on: May 10, 2015
 *      Author: Kevin
 */

#include "clear.h"

int clear_main(char* argv[], int argc)
{
	uint8_t outbuf[1] = {12};
	sys_write(STDOUT, outbuf, 1);
    return 0;
}
