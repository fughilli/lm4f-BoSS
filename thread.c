/*
 * thread.c
 *
 *  Created on: Mar 16, 2015
 *      Author: Kevin
 */

#include "thread.h"
#include <stdlib.h>
#include <string.h>

thread_t thread_table[MAX_THREADS];

thread_t* thread_current;
tid_t tid_counter;

uint8_t thread_mem[MAX_THREADS][THREAD_MEM_SIZE];

/**
 * Search the thread table for an entry with a matching pid
**/
static thread_t* tt_entry_for_tid(tid_t id)
{
	int i;
	for(i = 0; i < MAX_THREADS; i++)
	{
		if(thread_table[i].id == id)
		{
			return &thread_table[i];
		}
	}

	return (void*)0;
}

/**
 * Check if a given thread exists in the thread table
 */
static bool thread_valid(thread_t* thread)
{
	if(!thread)
		return false;

	int pos = (thread - thread_table);

	return (pos >= 0 && pos < MAX_THREADS);
}

/**
 * Init threads by setting them all to empty
 */
void thread_init(void)
{
	tid_counter = 0;
	int i;
	for(i = 0; i < MAX_THREADS; i++)
	{
		thread_table[i].id = 0;
		thread_table[i].state = T_EMPTY;

		//memset(&thread_table[i].regs, 0, sizeof(registers_t));
//		for(j = 0; j < sizeof(registers_t); j++)
//		{
//			((char*)&thread_table[i].regs)[j] = 0;
//		}
	}
}

tid_t thread_spawn(void (*entry)(void*), void* arg)
{
	int i;
	for(i = 0; i < MAX_THREADS; i++)
	{
		if(thread_table[i].state == T_EMPTY)
		{
			thread_table[i].state = T_RUNNABLE;

			// tid_counter increments for every process spawned, but it cannot be zero
			++tid_counter;
			if(tid_counter == 0)
				++tid_counter;

			// Assign the tid
			thread_table[i].id = tid_counter;

			// Clear the registers struct to all 0's
			// memset(&thread_table[i].regs, 0, sizeof(registers_t));

			thread_table[i].regs.PC = (uint32_t)entry;
			thread_table[i].regs.R0 = (uint32_t)arg;
			thread_table[i].regs.SP = (uint32_t)&thread_mem[i+1];

			return thread_table[i].id;
		}
	}

	// Invalid tid; could not spawn
	return 0;
}

bool thread_kill(thread_t* thread)
{
	//if(!(thread = tt_entry_for_tid(thread->id)))
	if(!thread_valid(thread))
		return false;

	thread->state = T_EMPTY;
	return true;
}

bool thread_kill2(tid_t tid)
{
	thread_t* thread;

	if(!(thread = tt_entry_for_tid(tid)))
		return false;

	thread->state = T_EMPTY;
	return true;
}

