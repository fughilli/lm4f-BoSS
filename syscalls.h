/*
 * syscalls.h
 *
 *  Created on: Mar 17, 2015
 *      Author: Kevin
 */

#ifndef SYSCALLS_H_
#define SYSCALLS_H_

#include "thread.h"
#include "syscall_numbers.h"
#include "file.h"
#include <stdint.h>
#include "fast_utils.h"

//			"mov R0,#0\n\t"
//			"mov R1,#1\n\t"
//			"mov R2,#2\n\t"
//			"mov R3,#3\n\t"
//			"mov R4,#4\n\t"
//			"mov R5,#5\n\t"
//			"mov R6,#6\n\t"
//			"mov R7,#7\n\t"
//			"mov R8,#8\n\t"
//			"mov R9,#9\n\t"
//			"mov R10,#10\n\t"
//			"mov R11,#11\n\t"
//			"mov R12,#12\n\t"

inline tid_t sys_get_tid()
{
	tid_t ret;
	asm volatile (
			"mov R0,%1\n\t"
			"svc $0x80\n\t"
			"mov %0,R0"
			: "=r" (ret) : "r" (SYSCALL_GET_TID) : "memory"
	);
	return ret;
}

inline tpri_t sys_get_pri()
{
	tpri_t ret;
	asm volatile (
			"mov R0,%1\n\t"
			"svc $0x80\n\t"
			"mov %0,R0"
			: "=r" (ret) : "r" (SYSCALL_GET_PRI) : "memory"
	);
	return ret;
}

inline void sys_set_pri(tpri_t pri)
{
	asm volatile (
			"mov R0,%0\n\t"
			"mov R1,%1\n\t"
			"svc $0x80\n\t"
			: : "r" (SYSCALL_SET_PRI), "r" (pri) : "memory"
	);
}

inline void sys_yield()
{
	asm volatile (
			"mov R0,%0\n\t"
			"svc $0x80"
			: : "r" (SYSCALL_YIELD) : "memory"
	);
}

inline int32_t sys_wait(fd_t fd)
{
	int32_t ret;
	asm volatile (
			"mov R0,%1\n\t"
			"mov R1,%2\n\t"
			"svc $0x80\n\t"
			"mov %0,R0"
			: "=r" (ret) : "r" (SYSCALL_WAIT), "r" (fd) : "memory"
	);
	return ret;
}

inline bool sys_kill(tid_t tid)
{
	bool ret;
	asm volatile (
			"mov R0,%1\n\t"
			"mov R1,%2\n\t"
			"svc $0x80\n\t"
			"mov %0,R0"
			: "=r" (ret) : "r" (SYSCALL_KILL), "r" (tid) : "memory"
	);
	return ret;
}

inline void sys_putc(char c)
{
	asm volatile (
			"mov R0,%0\n\t"
			"mov R1,%1\n\t"
			"svc $0x80"
			: : "r" (SYSCALL_PUTC), "r" ((uint32_t)c) : "memory"
	);
}

inline uint32_t sys_puts(const char* str, uint32_t len)
{
	uint32_t writelen;
	asm volatile (
			"mov R0,%1\n\t"
			"mov R1,%2\n\t"
			"mov R2,%3\n\t"
			"svc $0x80\n\t"
			"mov %0,R0"
			: "=r" (writelen) :
			"r" (SYSCALL_PUTS), "r" ((uint32_t)str), "r" (len) : "memory"
	);
	return writelen;
}

inline bool sys_lock(lock_t* l)
{
	bool ret;
	asm volatile (
			"mov R0,%1\n\t"
			"mov R1,%2\n\t"
			"svc $0x80\n\t"
			"mov %0,R0"
			: "=r" (ret) :
			"r" (SYSCALL_LOCK), "r" ((uint32_t)l) : "memory"
	);
	return ret;
}

inline void sys_unlock(lock_t* l)
{
	asm volatile (
			"mov R0,%0\n\t"
			"mov R1,%1\n\t"
			"svc $0x80"
			: : "r" (SYSCALL_UNLOCK), "r" ((uint32_t)l) : "memory"
	);
}

inline uint32_t sys_sleep(uint32_t ms)
{
    uint32_t ret_ms;
    asm volatile (
            "mov R0,%1\n\t"
            "mov R1,%2\n\t"
            "svc $0x80\n\t"
            "mov %0,R0"
            : "=r" (ret_ms) : "r" (SYSCALL_SLEEP), "r" (ms) : "memory"
    );
    return ret_ms;
}

inline uint32_t sys_fork()
{
    tid_t ret;
	asm volatile (
			"mov R0,%1\n\t"
			"svc $0x80\n\t"
			"mov %0,R0"
			: "=r" (ret) : "r" (SYSCALL_FORK) : "memory"
	);
	return ret;
}

inline void sys_set_port_dirs(uint32_t portbase, uint32_t dirs)
{
	asm volatile (
			"mov R0,%0\n\t"
			"mov R1,%1\n\t"
			"mov R2,%2\n\t"
			"svc $0x80"
			: :
			"r" (SYSCALL_SET_PORT_DIRS), "r" (portbase), "r" (dirs) : "memory", "0", "1", "2"
	);
}

inline void sys_write_port(uint32_t portbase, uint32_t portmask, uint32_t levels)
{
	asm volatile (
			"mov R0,%0\n\t"
			"mov R1,%1\n\t"
			"mov R2,%2\n\t"
			"mov R3,%3\n\t"
			"svc $0x80"
			: :
			"r" (SYSCALL_WRITE_PORT), "r" (portbase), "r" (portmask), "r" (levels) : "memory", "0", "1", "2", "3"
	);
}

inline uint32_t sys_read_port(uint32_t portbase)
{
	uint32_t ret;
	asm volatile (
			"mov R0,%1\n\t"
			"mov R1,%2\n\t"
			"svc $0x80\n\t"
			"mov %0,R0"
			: "=r" (ret) : "r" (SYSCALL_READ_PORT), "r" (portbase) : "memory", "0", "1"
	);
	return ret;
}

inline void sys_close(fd_t fd)
{
	asm volatile (
			"mov R0,%0\n\t"
			"mov R1,%1\n\t"
			"svc $0x80"
			: : "r" (SYSCALL_CLOSE), "r" (fd) : "memory", "0", "1"
	);
}

inline int32_t sys_read(fd_t fd, uint8_t* buf, int32_t len)
{
	int32_t ret;
	asm volatile (
			"mov R0,%1\n\t"
			"mov R1,%2\n\t"
			"mov R2,%3\n\t"
			"mov R3,%4\n\t"
			"svc $0x80\n\t"
			"mov %0,R0"
			: "=r" (ret) :
			"r" (SYSCALL_READ), "r" (fd), "r" (buf), "r" (len) : "memory", "0", "1", "2", "3"
	);
	return ret;
}

inline int32_t sys_write(fd_t fd, const uint8_t* buf, int32_t len)
{
	int32_t ret;
	asm volatile (
			"mov R0,%1\n\t"
			"mov R1,%2\n\t"
			"mov R2,%3\n\t"
			"mov R3,%4\n\t"
			"svc $0x80\n\t"
			"mov %0,R0"
			: "=r" (ret) :
			"r" (SYSCALL_WRITE), "r" (fd), "r" (buf), "r" (len) : "memory", "0", "1", "2", "3"
	);
	return ret;
}

inline int32_t sys_seek(fd_t fd, int32_t pos)
{
	int32_t ret;
	asm volatile (
			"mov R0,%1\n\t"
			"mov R1,%2\n\t"
			"mov R2,%3\n\t"
			"svc $0x80\n\t"
			"mov %0,R0"
			: "=r" (ret) :
			"r" (SYSCALL_SEEK), "r" (fd), "r" (pos) : "memory", "0", "1", "2", "3"
	);
	return ret;
}

inline uint32_t sys_ioctl(fd_t fd, uint32_t mask, void* arg)
{
	uint32_t ret;
	asm volatile (
			"mov R0,%1\n\t"
			"mov R1,%2\n\t"
			"mov R2,%3\n\t"
			"mov R3,%4\n\t"
			"svc $0x80\n\t"
			"mov %0,R0"
			: "=r" (ret) :
			"r" (SYSCALL_IOCTL), "r" (fd), "r" (mask), "r" (arg) : "memory", "0", "1", "2", "3"
	);
	return ret;
}

inline fd_t sys_open(const char* fname, fmode_t mode, fflags_t flags)
{
	fd_t ret;
	asm volatile (
			"mov R0,%1\n\t"
			"mov R1,%2\n\t"
			"mov R2,%3\n\t"
			"mov R3,%4\n\t"
			"svc $0x80\n\t"
			"mov %0,R0"
			: "=r" (ret) :
			"r" (SYSCALL_OPEN), "r" (fname), "r" (mode), "r" (flags) : "memory", "0", "1", "2", "3"
	);
	return ret;
}

inline bool sys_listdir(char* fnamebuf, size_t fnamebuflen)
{
	bool ret;
	asm volatile (
			"mov R0,%1\n\t"
			"mov R1,%2\n\t"
			"mov R2,%3\n\t"
			"svc $0x80\n\t"
			"mov %0,R0"
			: "=r" (ret) :
			"r" (SYSCALL_LISTDIR), "r" ((uint32_t)fnamebuf), "r" (fnamebuflen) : "memory", "0", "1", "2"
	);
	return ret;
}

inline void sys_rwdir()
{
	asm volatile (
			"mov R0,%0\n\t"
			"svc $0x80"
			: : "r" (SYSCALL_RWDIR) : "memory"
	);
}

inline bool sys_chdir(const char* dirname)
{
	bool ret;
	asm volatile (
			"mov R0,%1\n\t"
			"mov R1,%2\n\t"
			"svc $0x80\n\t"
			"mov %0,R0"
			: "=r" (ret) :
			"r" (SYSCALL_CHDIR), "r" ((uint32_t)dirname) : "memory", "0", "1"
	);
	return ret;
}

inline bool sys_mkdir(const char* dirname)
{
	bool ret;
	asm volatile (
			"mov R0,%1\n\t"
			"mov R1,%2\n\t"
			"svc $0x80\n\t"
			"mov %0,R0"
			: "=r" (ret) :
			"r" (SYSCALL_MKDIR), "r" ((uint32_t)dirname) : "memory", "0", "1"
	);
	return ret;
}

inline bool sys_unlink(const char* fname)
{
	bool ret;
	asm volatile (
			"mov R0,%1\n\t"
			"mov R1,%2\n\t"
			"svc $0x80\n\t"
			"mov %0,R0"
			: "=r" (ret) :
			"r" (SYSCALL_UNLINK), "r" ((uint32_t)fname) : "memory", "0", "1"
	);
	return ret;
}

inline tid_t sys_spawn(void (*entry)(void*), void* arg)
{
	tid_t ret;
	asm volatile (
			"mov R0,%1\n\t"
			"mov R1,%2\n\t"
			"mov R2,%3\n\t"
			"svc $0x80\n\t"
			"mov %0,R0"
			: "=r" (ret) :
			"r" (SYSCALL_SPAWN), "r" (entry), "r" (arg) : "memory", "0", "1", "2", "3"
	);
	return ret;
}


__attribute__((noreturn()))
inline void sys_exit(int status)
{
	asm volatile (
			"mov R0,%0\n\t"
			"mov R1,%1\n\t"
			"svc $0x80\n\t"
			: : "r" (SYSCALL_EXIT), "r" (status) : "memory"
	);

	while (1)
		;
}

__attribute__((noreturn()))
inline void sys_reset()
{
	asm volatile (
			"mov R0,%0\n\t"
			"svc $0x80\n\t"
			: : "r" (SYSCALL_RESET) : "memory"
	);

	while (1)
		;
}

void _exit(int status)
{
	sys_exit(0);

	while(1)
		;
}

#endif /* SYSCALLS_H_ */
