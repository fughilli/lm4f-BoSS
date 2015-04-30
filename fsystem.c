/*
 * fsystem.c
 *
 *  Created on: Apr 23, 2015
 *      Author: Kevin
 */

#include "fsystem.h"

fsys_assoc_t filesystem_table[MAX_FILESYSTEMS];

#define CALL_IF_VALID(_fptr_, ...) if((_fptr_)){(_fptr_)(__VA_ARGS__);}
#define CALLRET_IF_VALID(_fptr_, ...) if((_fptr_)){return (_fptr_)(__VA_ARGS__);}

void unmount(fsys_t fsys)
{
	CALL_IF_VALID(filesystem_table[0].funmap->unmount, fsys);
}

bool listdir(char* fnamebuf, size_t fnamebuflen)
{
	if (!fnamebuf || fnamebuflen == 0)
		return false;

	CALLRET_IF_VALID(filesystem_table[0].funmap->listdir, fnamebuf, fnamebuflen);

	return false;
}

void rwdir(void)
{
	CALL_IF_VALID(filesystem_table[0].funmap->rwdir);
}

fd_t open(const char* fname, fmode_t mode, fflags_t flags)
{
	if (!fname || !fast_strlen(fname))
		return FD_INVALID;

	CALLRET_IF_VALID(filesystem_table[0].funmap->open, fname, mode, flags);

	return FD_INVALID;
}

bool chdir(const char* dirname)
{
	if (!dirname || !fast_strlen(dirname))
		return false;

	CALLRET_IF_VALID(filesystem_table[0].funmap->chdir, dirname);

	return false;
}

bool mkdir(const char* dirname)
{
	if (!fast_strlen(dirname))
		return false;

	CALLRET_IF_VALID(filesystem_table[0].funmap->mkdir, dirname);

	return false;
}

bool unlink(const char* fname)
{
	if (!fast_strlen(fname))
		return false;

	CALLRET_IF_VALID(filesystem_table[0].funmap->unlink, fname);

	return false;
}
