/*
 * file.c
 *
 *  Created on: Mar 26, 2015
 *      Author: Kevin
 */

#include "file.h"

fd_assoc_t* file_table;

const fd_funmap_t file_funmap =
{
		.close = file_close,
		.read = file_read,
		.write = file_write
};

#define FD_VALID(_fd_) ((_fd_) >= 0 && (_fd_) < MAX_FILES)

void close(fd_t fd)
{
	if (!FD_VALID(fd))
		return;

	file_table[fd].funmap->close(fd);
}

int32_t read(fd_t fd, uint8_t* buf, int32_t len)
{
	if (!FD_VALID(fd))
		return RW_INVALID;

	return file_table[fd].funmap->read(fd, buf, len);
}

int32_t write(fd_t fd, uint8_t* buf, int32_t len)
{
	if (!FD_VALID(fd))
		return RW_INVALID;

	return file_table[fd].funmap->write(fd, buf, len);
}

fd_t file_open(const char* fname, uint32_t mode, uint32_t flags)
{
	return FD_INVALID;
}

void file_close(fd_t fd)
{

}

int32_t file_read(fd_t fd, uint8_t* buf, int32_t len)
{
	return RW_INVALID;
}

int32_t file_write(fd_t fd, uint8_t* buf, int32_t len)
{
	return RW_INVALID;
}
