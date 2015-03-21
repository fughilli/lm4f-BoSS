/*
 * thread.h
 *
 *  Created on: Mar 16, 2015
 *      Author: Kevin
 */

/**
 * Thread ID 0 is reserved
 */

#ifndef THREAD_H_
#define THREAD_H_

#include <stdint.h>
#include <stdbool.h>

#define MAX_THREADS (8)
#define THREAD_MEM_SIZE (1024)

typedef uint32_t tid_t;
typedef uint32_t tstate_t;
typedef uint32_t tpri_t;
typedef uint32_t tsleep_t;
typedef uint32_t lock_t;

#define LOCK_LOCKED (1)
#define LOCK_UNLOCKED (0)

enum
{
	T_EMPTY,

	T_RUNNABLE, T_BLOCKED, T_ZOMBIE, T_SLEEPING
};

typedef struct
{
	uint32_t R4;
	uint32_t R5;
	uint32_t R6;
	uint32_t R7;
	uint32_t R8;
	uint32_t R9;
	uint32_t R10;
	uint32_t R11;

	uint32_t SP;

	uint32_t R0;
	uint32_t R1;
	uint32_t R2;
	uint32_t R3;
	uint32_t R12;
	uint32_t LR;
	uint32_t PC;
	uint32_t PSR;
}__attribute__((aligned(0x4))) registers_t;

typedef struct
{
	// Thread ID
	tid_t id;

	// Thread state
	tstate_t state;

	// Thread priority
	tpri_t pri;

	// Thread sleep counter
	tsleep_t scnt;

	// Thread registers
	registers_t regs;
} thread_t;

extern thread_t thread_table[];
extern thread_t* thread_current;

void thread_init(void);
tid_t thread_spawn(void (*entry)(void*), void* arg);
bool thread_kill(thread_t* thread);
bool thread_kill2(tid_t tid);
bool thread_copy(thread_t* dest, const thread_t* src);
bool thread_fork(thread_t* thread);
bool thread_fork2(thread_t* thread, thread_t** rthread);

#endif /* THREAD_H_ */
