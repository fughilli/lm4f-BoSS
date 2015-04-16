/*
 * shell.h
 *
 *  Created on: Mar 28, 2015
 *      Author: Kevin
 */

#ifndef SHELL_H_
#define SHELL_H_

#include "osprogram.h"
#include <stdbool.h>

#define SHELL_LINE_BUFFER_SIZE (128)

#define SHELL_MAX_ARGS (8)
#define SHELL_MAX_PROGRAMS (16)
#define SHELL_MAX_PROGRAM_NAMELEN (16)

#define SHELL_BELL_CHAR (7)
#define SHELL_BACKSPACE (8)

typedef int(*shell_func_t)(char* argv[], int argc);

typedef struct
{
    char name[SHELL_MAX_PROGRAM_NAMELEN];
    shell_func_t prog_main;
} shell_progMapEntry_t;

void shell_main(void* arg);
void shell_initProgMap();
bool shell_registerProgram(const char* name, shell_func_t prog_main);

#endif /* SHELL_H_ */
