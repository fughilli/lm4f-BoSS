/*
 * pipe.c
 *
 *  Created on: Mar 26, 2015
 *      Author: Kevin
 */

#include "pipe.h"

const fd_funmap_t pipe_funmap =
{
		.close = pipe_close,
		.read = pipe_read,
		.write = pipe_write
};

fd_t pipe_create(size_t bufsiz)
{
	return FD_INVALID;
}

void pipe_close(fd_t fd)
{

}

int32_t pipe_read(fd_t fd, uint8_t* buf, int32_t len)
{
	return RW_INVALID;
}

int32_t pipe_write(fd_t fd, uint8_t* buf, int32_t len)
{
	return RW_INVALID;
}

