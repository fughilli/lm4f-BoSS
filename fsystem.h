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
#include "fast_utils.h"

typedef int32_t fsys_t;

#define MAX_FILESYSTEMS (2)

#define FSYS_INVALID ((int32_t)(-1))

#define FSYS_VALID(_fsys_) (((_fsys_) >= 0) && ((_fsys_) < MAX_FILES) && (filesystem_table[(_fsys_)].funmap))

typedef struct
{
	void (*unmount)(fsys_t);
	bool (*listdir)(char*, size_t);
	void (*rwdir)(void);
	fd_t (*open)(const char*,fmode_t,fflags_t);
	bool (*chdir)(const char*);
	bool (*unlink)(const char*);
	bool (*mkdir)(const char*);
} fsys_funmap_t;

typedef struct
{
	void* fsysdata;
	const fsys_funmap_t* funmap;
	const char* mountpoint;
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
fd_t open(const char* fname, fmode_t mode, fflags_t flags);
bool listdir(char* fnamebuf, size_t fnblen);
void rwdir(void);
void unmount(fsys_t fsys);
bool chdir(const char* dirname);
bool mkdir(const char* dirname);
bool unlink(const char* fname);

#endif /* FSYSTEM_H_ */
