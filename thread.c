/*
 * thread.c
 *
 *  Created on: Mar 16, 2015
 *      Author: Kevin
 */

#include "thread.h"
#include "fast_utils.h"

thread_t thread_table[MAX_THREADS] __attribute__((aligned(0x4)));

thread_t* thread_current;
tid_t tid_counter;

// Needs to be aligned on the size of the thread memory region
// so that the MPU can be used to protect it
uint8_t thread_mem[MAX_THREADS][THREAD_MEM_SIZE] __attribute__((aligned(THREAD_MEM_SIZE)));

// stdin, stdout, sderr are exceptions
#define THREADFD_VALID(_tfd_) ((_tfd_) >= 0 && (_tfd_) < (THREAD_MAX_OPEN_FDS))
#define THREADFD_VALID_AND_NOT_STD(_tfd_) ((_tfd_) > 2 && (_tfd_) < (THREAD_MAX_OPEN_FDS))

/**
 * Search the thread table for an entry with a matching pid
**/
thread_t* tt_entry_for_tid(tid_t id)
{
	int i;
	for(i = 0; i < MAX_THREADS; i++)
	{
		if(thread_table[i].id == id && thread_table[i].state != T_EMPTY)
		{
			return &thread_table[i];
		}
	}

	return (void*)0;
}

/**
 * Find the position of a thread in the thread table from a
 * pointer to that table entry
 */
uint32_t thread_pos(const thread_t* thread)
{
    if(!thread)
        return MAX_THREADS;

    int pos = (thread - thread_table);

    if(pos >= 0 && pos < MAX_THREADS)
        return (uint32_t)pos;

    return MAX_THREADS;
}

/**
 * Find the first free slot in the thread table. If there
 * are no free spots, return MAX_THREADS
 */
static uint32_t thread_first_empty()
{
    uint32_t ret = 0;
    do
    {
        if(thread_table[ret].state == T_EMPTY)
            return ret;
    } while((++ret) < MAX_THREADS);

    return ret;
}

/**
 * Check if a given thread exists in the thread table
 */
bool thread_valid(const thread_t* thread)
{
	return (thread_pos(thread) != MAX_THREADS);
}

/**
 * Init threads by setting them all to empty
 */
void thread_init(void)
{
	tid_counter = 0;
	int i, j;
	for(i = 0; i < MAX_THREADS; i++)
	{
		thread_table[i].id = 0;
		thread_table[i].state = T_EMPTY;

		thread_table[i].wait_func = NULL;

		for(j = 0; j < THREAD_MAX_OPEN_FDS; j++)
		{
			thread_table[i].open_fds[j] = FD_INVALID;
		}

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

    // If there are no free thread spots, return invalid
    if((i = thread_first_empty()) == MAX_THREADS)
        return 0;

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

    // Ensure that thumb state is enabled
    thread_table[i].regs.PSR = 0x01000000;

    thread_table[i].pri = 1;
    thread_table[i].scnt = 0;

    // Setup the file descriptor table to invalid fds
    int j;
	for (j = 0; j < THREAD_MAX_OPEN_FDS; j++)
	{
		thread_table[i].open_fds[j] = FD_INVALID;
	}

    return thread_table[i].id;
}

fd_t thread_get_free_fd(thread_t* thread)
{
	int threadfd;
	// Skip over stdin, stdout, stderr
	for(threadfd = 3; threadfd < THREAD_MAX_OPEN_FDS; threadfd++)
	{
		if(thread->open_fds[threadfd] == FD_INVALID)
			return threadfd;
	}
	return FD_INVALID;
}

fd_t thread_alloc_fd(thread_t* thread, fd_t sysfd)
{
	if(!FD_VALID(sysfd))
		return FD_INVALID;

	int freefd;
	if((freefd = thread_get_free_fd(thread)) != FD_INVALID)
	{
		thread->open_fds[freefd] = sysfd;
		return freefd;
	}
	return FD_INVALID;
}

fd_t thread_free_fd(thread_t* thread, fd_t threadfd)
{
	if(!THREADFD_VALID_AND_NOT_STD(threadfd))
		return FD_INVALID;

	fd_t sysfd = thread->open_fds[threadfd];
	thread->open_fds[threadfd] = FD_INVALID;
	return sysfd;
}

fd_t thread_lookup_fd(thread_t* thread, fd_t threadfd)
{
	if(!THREADFD_VALID(threadfd))
		return FD_INVALID;

	fd_t sysfd = thread->open_fds[threadfd];
	return sysfd;
}

tid_t thread_spawn2(void (*entry)(void*), void* arg, fd_t stdin, fd_t stdout, fd_t stderr)
{
	tid_t res = thread_spawn(entry, arg);

	// If the thread was spawned
	if(res)
	{
		thread_t* thread = tt_entry_for_tid(res);
		thread->open_fds[0] = stdin;
		thread->open_fds[1] = stdout;
		thread->open_fds[2] = stderr;
	}

	return res;
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

bool thread_copy(thread_t* dest, const thread_t* src)
{
    if(!thread_valid(dest) || !thread_valid(src) ||
       dest->state != T_EMPTY || src->state == T_EMPTY)
        return false;

    // Get the table entry locations for the destination and source
    uint32_t d_index = thread_pos(dest), s_index = thread_pos(src);

    // Assign the cloned thread a new tid
    ++tid_counter;
    if(tid_counter == 0)
        ++tid_counter;

    fast_memcpy(dest, src, sizeof(thread_t));
    fast_memcpy(thread_mem[d_index], thread_mem[s_index], THREAD_MEM_SIZE);

    thread_table[d_index].id = tid_counter;

    return true;
}

bool thread_fork(thread_t* thread)
{
    return thread_fork2(thread, (thread_t**)0);
}

bool thread_fork2(thread_t* thread, thread_t** rt)
{
    if(!thread_valid(thread))
        return false;

    // Find a free slot
    uint32_t destpos = thread_first_empty();

    if(destpos == MAX_THREADS)
        return false;

    // Copy the thread, giving it a new tid; pass back the return status
    // and the thread table entry
    if(thread_copy(&thread_table[destpos], thread))
    {
        if(rt)
            (*rt) = &thread_table[destpos];
        return true;
    }

    return false;
}

// Find all blocked threads waiting on the arg, thread, and wake them up
// Also pass them the return value of the arg
void thread_notify_waiting(thread_t* thread)
{
	if(!thread_valid(thread))
		return;

	int i;

	for(i = 0; i < MAX_THREADS; i++)
	{
		if((thread_table[i].state == T_BLOCKED) &&
		   (thread_table[i].wait_func == WAITING_ON_THREAD) &&
		   (((tid_t)thread_table[i].regs.R1) == thread->id))
		{
			thread_table[i].regs.R0 = thread->regs.R1;
			thread_table[i].state = T_RUNNABLE;
		}
	}
}

// Find all blocked threads waiting on the arg, file, and wake them up
// Reattempt the read, and pass them the return value
void thread_notify_waiting_readers(fd_t fd)
{
	if(!FD_VALID(fd))
		return;

		int i;

		for(i = 0; i < MAX_THREADS; i++)
		{
			// If the thread is blocked on a file
			if((thread_table[i].state == T_BLOCKED) &&
			   (thread_table[i].wait_func == WAITING_ON_FILE))
			{
				// Lookup the sysfd of the fd it is waiting on
				fd_t sysfd = thread_lookup_fd(&thread_table[i], (fd_t) thread_table[i].regs.R1);
				// Is it waiting on the fd whose readers are being notified?
				if(sysfd == fd)
				{
					// Perform the read again
					int32_t readsiz = read(
							sysfd,
							(uint8_t*) thread_table[i].regs.R2,
							(int32_t) thread_table[i].regs.R3);

					// If (this time) some bytes were read, copy over the return
					// value, and set the waiting thread runnable
					if(readsiz > 0)
					{
						thread_table[i].regs.R0 = (uint32_t)readsiz;
						thread_table[i].state = T_RUNNABLE;
					}
				}
			}
		}
}
