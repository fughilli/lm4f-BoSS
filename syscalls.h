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
#include <stdint.h>

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

inline void sys_putc(char c)
{
	asm volatile (
			"mov R0,%0\n\t"
			"mov R1,%1\n\t"
			"svc $0x80"
			: : "r" (SYSCALL_PUTC), "r" ((uint32_t)c) : "memory"
	);
}

inline uint32_t sys_puts(char* str, uint32_t len)
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

//inline void sys_set_port_dirs(uint32_t portbase, uint8_t dirs)
//{
//
//}
//
//inline void sys_write_port(uint32_t portbase, uint8_t portmask, uint8_t levels)
//{
//
//}
//
//inline uint8_t sys_read_port(uint32_t portbase)
//{
//
//}

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

#endif /* SYSCALLS_H_ */
