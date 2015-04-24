/*
 * fsystem.h
 *
 *  Created on: Apr 23, 2015
 *      Author: Kevin
 */

#ifndef FSYSTEM_H_
#define FSYSTEM_H_

#include <stdint.h>
#include <stdbool.h>
#include "file.h"

typedef int32_t fsys_t;

#define MAX_FILESYSTEMS (2)

#define FSYS_INVALID ((int32_t)(-1))

#define FSYS_VALID(_fsys_) (((_fsys_) >= 0) && ((_fsys_) < MAX_FILES) && (filesystem_table[(_fsys_)].funmap))

typedef struct
{
	void (*unmount)(fsys_t);
	bool (*listdir)(char*, uint32_t);
	bool (*touch)(const char*);
	fd_t (*open)(const char*,fmode_t,fflags_t);
} fsys_funmap_t;

typedef struct
{
	void* fsysdata;
	const fsys_funmap_t* funmap;
} fsys_assoc_t;

// The filesystem table; holds all open fsys associations
extern fsys_assoc_t filesystem_table[MAX_FILESYSTEMS];

fd_t fsystable_getfree();
void fsystable_free(fsys_t);
void fsystable_init();

// Top-level functions for manipulating "files";
// Any device modeled as a file is operated upon
// by these functions. All "files" have an
// associated file descriptor (fd) when they are
// open. This fd is allocated by the *_open
// functions
void close(fd_t fd);
fd_t open(const char* fname, fmode_t mode, fflags_t flags);
bool touch(const char* fname);
bool listdir(char* fnamebuf, uint32_t fnblen);
void unmount(fsys_t fsys);

#endif /* FSYSTEM_H_ */
