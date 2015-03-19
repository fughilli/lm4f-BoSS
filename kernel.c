/*
 * kernel.c
 *
 *  Created on: Mar 16, 2015
 *      Author: Kevin
 */

#include "kernel.h"
#include "thread.h"

void kernel_schedule();

void kernel_init(void (*first_proc_main)(void*))
{
	thread_init();

	thread_table[0].state = T_RUNNABLE;
	thread_table[0].id = 0;
	thread_current = &thread_table[0];

	asm volatile (
			"mov R0,%0\n\t"
			"svc $0x80"
			: : "r" (SYSCALL_YIELD) : "memory"
	);
}

__attribute__((noreturn))
void kernel_schedule()
{
	int i;
	for (i = 0; i < MAX_THREADS; i++)
	{
		if (thread_table[i].id == thread_current->id)
			break;
	}

	i++;
	if (i == MAX_THREADS)
		i = 0;

	while (1)
	{
		for (; i < MAX_THREADS; i++)
		{
			if (thread_table[i].state == T_RUNNABLE)
			{
				kernel_run(&thread_table[i]);
			}
		}

		i = 0;
	}
}

__attribute__((noreturn))
void kernel_run(thread_t* thread)
{
	thread_current = thread;
	asm volatile (
			"mov SP,%0\n\t"
			"pop {R0,R1,R2,R3,R4,R5,R6,R7,R8,R9,R10,R11,R12}\n\t"
			// See page 735 of the user guide
			// Load the status bits from the stack
			"ldr LR,[SP]\n\t"
			"msr APSR_nczvq,LR\n\t"

			// Move the stack pointer (simulate POP to APSR)
			"add SP,#4\n\t"

			// Pop return address into LR
			"pop {LR}\n\t"

			// Pop stack pointer
			"ldr SP,[SP]\n\t"

			// Jump
			"mov LR,$0xFFFFFFF9\n\t"
			"bx LR"
			: : "r" (&thread_current->regs) : "memory" );

	while (1)
		;
}

// Called by SVC
//__attribute__((noreturn))
void svc_interrupt_handler()
{
	// Push the context of the trapping thread
	asm volatile (
			// Push the remaining registers;
			// R0,R1,R2,R3,R12,xPSR are pushed
			// R7 is pushed on call entry
			// Remaining registers are:
			"push {R4, R5, R6, R8, R9, R10, R11}\n\t"
			// Save the stack pointer to SP
			"mov %0,SP"
			: "=r" (thread_current->regs.SP) : : "memory"
	);

	// Load the status bits into LR, then store them into the struct
	asm volatile (
			"mrs LR,APSR\n\t"
			"mov %0,LR"
			: "=r" (thread_current->regs.PSR) : : "memory"
	);

	uint32_t* oldsp = (uint32_t*) thread_current->regs.SP;
	thread_current->regs.R4 = oldsp[0];
	thread_current->regs.R5 = oldsp[1];
	thread_current->regs.R6 = oldsp[2];
	thread_current->regs.R8 = oldsp[3];
	thread_current->regs.R9 = oldsp[4];
	thread_current->regs.R10 = oldsp[5];
	thread_current->regs.R11 = oldsp[6];

	thread_current->regs.R7 = oldsp[9];

	thread_current->regs.R0 = oldsp[11];
	thread_current->regs.R1 = oldsp[12];
	thread_current->regs.R2 = oldsp[13];
	thread_current->regs.R3 = oldsp[14];
	thread_current->regs.R12 = oldsp[15];
	thread_current->regs.PC = oldsp[16];

	switch (thread_current->regs.R0)
	{

	// Get the thread ID of the calling process
	case SYSCALL_GET_TID:
		thread_current->regs.R0 = thread_current->id;
		kernel_run(thread_current);
		break;

	case SYSCALL_EXIT:
		thread_kill(thread_current);
		kernel_schedule();
		break;

	case SYSCALL_YIELD:
		kernel_schedule();
		break;

	case SYSCALL_SET_PRI:
		thread_current->pri = thread_current->regs.R1;
		kernel_run(thread_current);
		break;

	case SYSCALL_GET_PRI:
		thread_current->regs.R0 = thread_current->pri;
		kernel_run(thread_current);
		break;

	case SYSCALL_SET_PORT_DIRS:
		//SysCtlPeripheralEnable(GPIO)
		kernel_run(thread_current);
		break;

	case SYSCALL_PUTC:
		// call low-level IO send function
		// argument is thread_current->regs.R1
		kernel_run(thread_current);
		break;
	}

	while (1)
		;
}

void systick_interrupt_handler()
{
	// Push the context of the trapping thread
	asm volatile (
			// Push the remaining registers;
			// R0,R1,R2,R3,R12,xPSR are pushed
			// R7 is pushed on call entry
			// Remaining registers are:
			"push {R4, R5, R6, R8, R9, R10, R11}\n\t"
			// Save the stack pointer to SP
			"mov %0,SP"
			: "=r" (thread_current->regs.SP) : : "memory"
	);

	// Load the status bits into LR, then store them into the struct
	asm volatile (
			"mrs LR,APSR\n\t"
			"mov %0,LR"
			: "=r" (thread_current->regs.PSR) : : "memory"
	);

	uint32_t* oldsp = (uint32_t*) thread_current->regs.SP;
	thread_current->regs.R4 = oldsp[0];
	thread_current->regs.R5 = oldsp[1];
	thread_current->regs.R6 = oldsp[2];
	thread_current->regs.R8 = oldsp[3];
	thread_current->regs.R9 = oldsp[4];
	thread_current->regs.R10 = oldsp[5];
	thread_current->regs.R11 = oldsp[6];

	thread_current->regs.R7 = oldsp[9];

	thread_current->regs.R0 = oldsp[11];
	thread_current->regs.R1 = oldsp[12];
	thread_current->regs.R2 = oldsp[13];
	thread_current->regs.R3 = oldsp[14];
	thread_current->regs.R12 = oldsp[15];
	thread_current->regs.PC = oldsp[16];

	kernel_schedule();
}

void _exit(int status)
{
	while (1)
		;
}
