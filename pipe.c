/*
 * pipe.c
 *
 *  Created on: Mar 26, 2015
 *      Author: Kevin
 */

#include "pipe.h"
#include "fast_utils.h"

const fd_funmap_t pipe_funmap =
{
		.close = pipe_close,
		.read = pipe_read,
		.write = pipe_write,
		.seek = NULL,
		.ioctl = NULL,
		.rem = pipe_rem
};

fd_t ATOMIC pipe_create(size_t bufsiz)
{
	uint8_t* buf;
	fd_t pfd;
	pipe_t* ps;

	// Find a free file_table entry
	if((pfd = ftable_getfree()) == FD_INVALID)
		return FD_INVALID;

	// Allocate the pipe buffer
	if(!(buf = (uint8_t*)fast_alloc(bufsiz + sizeof(pipe_t))))
		return FD_INVALID;

	/**
	 * Set up the pipe_t
	 * Layout in memory:
	 *
	 * +---------------+-------------------------+
	 * | off=0: pipe_t | off=sizeof(pipe_t): buf |
	 * +---------------+-------------------------+
	 */
	ps = (pipe_t*)buf;
	ps->buf = &buf[sizeof(pipe_t)];
	ps->head = ps->tail = 0;
	ps->bufsiz = bufsiz;

	/**TODO: this is actually not needed, because
	 * file operations are executed by the kernel
	 * during an SVC and are therefore atomic.
	 */
	ps->lock = 0;

	// Set up the file_table entry
	file_table[pfd].fdata = (void*)ps;
	file_table[pfd].funmap = &pipe_funmap;

	return pfd;
}

//TODO: Some error checking here
void pipe_close(fd_t fd)
{
	if(!FD_VALID(fd))
		return;

	fast_free(file_table[fd].fdata);
	ftable_free(fd);
}

int32_t pipe_read(fd_t fd, uint8_t* buf, int32_t len)
{
	if(!FD_VALID_PIPE(fd))
		return RW_INVALID;

	pipe_t* ps = (pipe_t*)file_table[fd].fdata;
	//TODO: pipe_read--> while(!sys_lock(&ps->lock));

	int32_t oidx = 0;

	for(; len > 0 && ((ps->head) != ps->tail); len--)
	{
		buf[oidx++] = ps->buf[ps->head++];
		if(ps->head == ps->bufsiz)
			ps->head = 0;
	}

	//TODO: pipe_read--> sys_unlock(&ps->lock);

	return oidx;
}

int32_t pipe_rem(fd_t fd)
{
	if(!FD_VALID_PIPE(fd))
		return REM_INVALID;

	pipe_t* ps = (pipe_t*)file_table[fd].fdata;
	//TODO: pipe_rem--> while(!sys_lock(&ps->lock));

	if(ps->tail >= ps->head)
		return (int32_t)(ps->tail - ps->head);
	else
		return (int32_t)((ps->tail + ps->bufsiz) - ps->head);

	//TODO: pipe_rem--> sys_unlock(&ps->lock);

	return REM_INVALID;
}

int32_t pipe_write(fd_t fd, const uint8_t* buf, int32_t len)
{
	if (!FD_VALID_PIPE(fd))
		return RW_INVALID;

	pipe_t* ps = (pipe_t*) file_table[fd].fdata;
	//TODO: pipe_read--> while(!sys_lock(&ps->lock));

	int32_t iidx = 0;

	for (; len > 0 && ((ps->tail + 1) != ps->head); len--)
	{
		ps->buf[ps->tail++] = buf[iidx++];
		if (ps->tail == ps->bufsiz)
			ps->tail = 0;
	}

	//TODO: pipe_read--> sys_unlock(&ps->lock);

	return iidx;
}

int32_t pipe_transfer(fd_t destfd, fd_t srcfd, int32_t len)
{
	if (!FD_VALID_PIPE(destfd) || !FD_VALID_PIPE(srcfd))
		return RW_INVALID;

	pipe_t* srcps = (pipe_t*) file_table[srcfd].fdata;
	pipe_t* destps = (pipe_t*) file_table[destfd].fdata;

	int32_t bytes_transferred = 0;

	for (; len > 0 && ((srcps->head) != srcps->tail) && ((destps->tail + 1) != destps->head); len--)
	{
		destps->buf[destps->tail++] = srcps->buf[srcps->head++];
		if (destps->head == destps->bufsiz)
			destps->head = 0;
		if (srcps->tail == srcps->bufsiz)
			srcps->tail = 0;
		bytes_transferred++;
	}

	return bytes_transferred;
}

