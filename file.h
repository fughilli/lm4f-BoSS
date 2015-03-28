/*
 * file.h
 *
 *  Created on: Mar 25, 2015
 *      Author: Kevin
 */

#ifndef FILE_H_
#define FILE_H_

#include <stdint.h>
#include <stdbool.h>

typedef int32_t fd_t;

#define MAX_FILES (16)

#define FD_INVALID ((int32_t)(-1))
#define RW_INVALID ((int32_t)(-1))

#define FMODE_R (0x00000001)
#define FMODE_W (0x00000002)

#define FFLAG_CREAT 0x00000001
#define FFLAG_APPND 0x00000002
#define FFLAG_TRUNC 0x00000004
//#define FFLAG_OVERW

typedef struct
{
	void (*close)(fd_t);
	int32_t (*read)(fd_t, uint8_t*, int32_t);
	int32_t (*write)(fd_t, uint8_t*, int32_t);
} fd_funmap_t;

typedef struct
{
	fd_t fd;
	void* fdata;
	fd_funmap_t* funmap;
} fd_assoc_t;

// Method map for regular files
extern const fd_funmap_t file_funmap;

// The file table; holds all open fd associations
extern fd_assoc_t* file_table;

// Top-level functions for manipulating "files";
// Any device modeled as a file is operated upon
// by these functions. All "files" have an
// associated file descriptor (fd) when they are
// open. This fd is allocated by the *_open
// functions
void close(fd_t fd);
int32_t read(fd_t fd, uint8_t* buf, int32_t len);
int32_t write(fd_t fd, uint8_t* buf, int32_t len);

fd_t file_open(const char* fname, uint32_t mode, uint32_t flags);
void file_close(fd_t fd);

int32_t file_read(fd_t fd, uint8_t* buf, int32_t len);
int32_t file_write(fd_t fd, uint8_t* buf, int32_t len);

#endif /* FILE_H_ */
