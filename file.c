/*
 * file.c
 *
 *  Created on: Mar 26, 2015
 *      Author: Kevin
 */

#include "file.h"
#include "fast_utils.h"

#define CALL_IF_VALID(_fptr_, ...) if((_fptr_)){(_fptr_)(__VA_ARGS__);}
#define CALLRET_IF_VALID(_fptr_, ...) if((_fptr_)){return (_fptr_)(__VA_ARGS__);}

void ftable_init()
{
	fd_t i;
	for (i = 0; i < MAX_FILES; i++)
	{
		file_table[i].funmap = NULL;
	}
}

fd_t ftable_getfree()
{
	fd_t i;
	for (i = 0; i < MAX_FILES; i++)
	{
		if(file_table[i].funmap == NULL)
			return i;
	}

	return FD_INVALID;
}

void ftable_free(fd_t fd)
{
	if(FD_VALID(fd))
		file_table[fd].funmap = NULL;
}

void close(fd_t fd)
{
	if (!FD_VALID(fd))
		return;

//	if(file_table[fd].funmap->close)
//		file_table[fd].funmap->close(fd);

	CALL_IF_VALID(file_table[fd].funmap->close, fd);
}

int32_t read(fd_t fd, uint8_t* buf, int32_t len)
{
	if (!FD_VALID(fd))
		return RW_INVALID;

	CALLRET_IF_VALID(file_table[fd].funmap->read, fd, buf, len);

	return RW_INVALID;
}

int32_t write(fd_t fd, const uint8_t* buf, int32_t len)
{
	if (!FD_VALID(fd))
		return RW_INVALID;

	CALLRET_IF_VALID(file_table[fd].funmap->write, fd, buf, len);

		return RW_INVALID;
}

int32_t seek(fd_t fd, int32_t pos)
{
	if (!FD_VALID(fd))
			return RW_INVALID;

		CALLRET_IF_VALID(file_table[fd].funmap->seek, fd, pos);

			return SEEK_INVALID;
}

uint32_t ioctl(fd_t fd, uint32_t mask, void* arg)
{
	if (!FD_VALID(fd))
			return RW_INVALID;

		CALLRET_IF_VALID(file_table[fd].funmap->ioctl, fd, mask, arg);

			return IOCTL_INVALID;
}

int32_t rem(fd_t fd)
{
	if (!FD_VALID(fd))
		return RW_INVALID;

	CALLRET_IF_VALID(file_table[fd].funmap->rem, fd);

	return RW_INVALID;
}
