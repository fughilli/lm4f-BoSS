/*
 * pipe.h
 *
 *  Created on: Mar 25, 2015
 *      Author: Kevin
 */

#ifndef PIPE_H_
#define PIPE_H_

#include <stdint.h>
#include <stdbool.h>
#include "fast_utils.h"
#include "kernel.h"
#include "file.h"

typedef struct
{
	lock_t lock;
	uint32_t head, tail;
	uint8_t* buf;
	uint32_t bufsiz;
} pipe_t;

extern const fd_funmap_t pipe_funmap;

#define FD_VALID_PIPE(_fd_) (((_fd_) >= 0) && ((_fd_) < MAX_FILES) && (file_table[(_fd_)].funmap == &pipe_funmap))

fd_t pipe_create(size_t bufsiz);
void pipe_close(fd_t fd);

int32_t pipe_read(fd_t fd, uint8_t* buf, int32_t len);
int32_t pipe_write(fd_t fd, const uint8_t* buf, int32_t len);
int32_t pipe_transfer(fd_t destfd, fd_t srcfd, int32_t len);
int32_t pipe_rem(fd_t fd);

#endif /* PIPE_H_ */
