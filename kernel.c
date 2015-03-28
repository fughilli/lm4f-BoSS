/*
 * kernel.c
 *
 *  Created on: Mar 16, 2015
 *      Author: Kevin
 */

#include "kernel.h"
#include "thread.h"
#include "debug_serial.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_nvic.h"

#include <stdbool.h>

//uint8_t kernel_stack[128] __attribute((aligned(8)));

const char __k_kp_str_hdr[] = "Kernel panic: ";
const char __k_kp_str_hardfault[] = "hard fault";
const char __k_kp_str_nmi[] = "non-maskable interrupt";
const char __k_kp_str_default[] = "unimplemented ISR";
const char __k_kp_str_nl[] = "\r\n";

tsleep_t systime_ms;

void kernel_schedule();
static void kernel_set_scheduler_freq(uint32_t hz);
void kernel_assert(bool cond, const char* fstr, uint32_t fstrlen);

void kernel_init()
{
	thread_init();

	thread_table[0].state = T_RUNNABLE;
	thread_table[0].id = 0;
	thread_current = &thread_table[0];

	systime_ms = 0;

	int i;
	for(i = 0; i < MAX_THREADS; i++)
	{
		kernel_assert(thread_pos(&thread_table[i]) == i, "thread_pos: incorrect behavior", 100);
		kernel_assert(thread_valid(&thread_table[i]), "thread_valid: incorrect behavior", 100);
	}

	kernel_set_scheduler_freq(KERNEL_SCHEDULER_IRQ_FREQ);
}

__attribute__((noreturn))
void kernel_panic(const char* kpstr, uint32_t kpstrlen)
{
	Serial_puts(__k_kp_str_hdr, sizeof(__k_kp_str_hdr));
	Serial_puts(kpstr, kpstrlen);
	Serial_puts(__k_kp_str_nl, sizeof(__k_kp_str_nl));

	while(1)
		;
}

void kernel_assert(bool cond, const char* fstr, uint32_t fstrlen)
{
	if(!cond)
		kernel_panic(fstr, fstrlen);
}

__attribute__((noreturn))
void kernel_schedule()
{
	uint32_t i = thread_pos(thread_current);

	if(i == MAX_THREADS)
		kernel_panic("kernel_schedule: thread_current not in thread table", 51);

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
			"pop {R4,R5,R6,R7,R8,R9,R10,R11}\n\t"

			// Save the old SP, which points to the new process' SP
			"mov R0,SP\n\t"
			"add R0,R0,$32\n\t"

			// Pop stack pointer; SP now points to new process' stack top
			"ldr SP,[SP]\n\t"

			// Copy
			"mov R1,$8\n"
			"_loop_head:\n\t"
			"ldr R3,[R0]\n\t"
			"push {R3}\n\t"
			"add R0,R0,$-4\n\t"
			"add R1,R1,$-1\n\t"
			"cbz R1,_return_except\n\t"
			"b _loop_head\n"

			// Exception return
			"_return_except:\n\t"
			"mov LR,$0xFFFFFFF9\n\t"
			"bx LR"
			: : "r" (&thread_current->regs.R4) : "memory" );

	while (1)
		;
}

__attribute__((noreturn))
static void kernel_handle_syscall()
{
	thread_t* child_thread;
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

	case SYSCALL_FLUBBER:
		Serial_puts("Flubber!", 8);
		kernel_run(thread_current);
		break;

	case SYSCALL_PUTC:
		// call low-level IO send function
		// argument is thread_current->regs.R1
		Serial_putc(thread_current->regs.R1);
		kernel_run(thread_current);
		break;

	case SYSCALL_PUTS:
		Serial_puts((char*) thread_current->regs.R1, thread_current->regs.R2);
		kernel_run(thread_current);
		break;

	case SYSCALL_LOCK:
		// If the lock is already taken, return 0 and resume the spinlock
		// TODO: check for nullptr
		if (*((lock_t*) thread_current->regs.R1))
		{
			thread_current->regs.R0 = false;
		}
		else
		{
			// Otherwise, lock it
			*((lock_t*) thread_current->regs.R1) = LOCK_LOCKED;
			thread_current->regs.R0 = true;
		}
		kernel_run(thread_current);
		break;

	case SYSCALL_UNLOCK:
		// Release the lock
		*((lock_t*) thread_current->regs.R1) = LOCK_UNLOCKED;
		kernel_schedule();
		break;

	case SYSCALL_FORK:
		// Find a free thread slot and clone the thread into it
		if (thread_fork2(thread_current, &child_thread))
		{
			// Set the correct return values
			child_thread->regs.R0 = 0;
			thread_current->regs.R0 = child_thread->id;
		}
		else
		{
			thread_current->regs.R0 = 0;
		}
		kernel_run(thread_current);
		break;

	case SYSCALL_SLEEP:
		if (thread_current->regs.R1 > 0)
		{
			thread_current->scnt = thread_current->regs.R0 =
					(thread_current->regs.R1 / SYSTIME_CYCLES_PER_MS);
			thread_current->scnt += systime_ms;
			thread_current->state = T_SLEEPING;
			kernel_schedule();
		}
		else
		{
			thread_current->regs.R0 = 0;
			kernel_run(thread_current);
		}
		break;
	}

	kernel_panic("unknown syscall", 15);

	while(1)
		;

}

static void kernel_tick_counter(void)
{
	systime_ms++;

	int i;
	for (i = 0; i < MAX_THREADS; i++)
	{
		if (thread_table[i].state == T_SLEEPING)
		{
			if (systime_ms == thread_table[i].scnt)
			{
				thread_table[i].state = T_RUNNABLE;
			}
		}
	}
}

// Called by SVC
__attribute__((noreturn))
void svc_interrupt_handler()
{
	// Push the context of the trapping thread
	asm volatile (
			"dsb\n\t"
			"isb\n\t"
			// Push the remaining registers;
			// R0,R1,R2,R3,R12,LR,PC+4,xPSR are pushed
			// R7 is pushed on call entry
			// Remaining registers are:
			"push {R4, R5, R6, R8, R9, R10, R11}\n\t"
			// Save the stack pointer to SP
			"mov %0,SP"
			: "=r" (thread_current->regs.SP) : : "memory"
	);

	//asm volatile("mov SP,%0" : : "r" (kernel_stack) : "memory");

	uint32_t* oldsp = (uint32_t*) thread_current->regs.SP;

	// Adjust the saved stack pointer to sit above the pushed exception frame
	//thread_current->regs.SP += 0x50;
	thread_current->regs.SP += 0x4C;

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
	thread_current->regs.LR = oldsp[16];
	thread_current->regs.PC = oldsp[17];
	thread_current->regs.PSR = oldsp[18];

	/*
	 * Determine exception cause and tick the counter/jump to schedule if SysTick
	 */
	asm volatile(
			"mrs R12,XPSR\n\t"
			"lsl R12,#23\n\t"
			"lsr R12,#23\n\t"
			"sub R5,R12,#15\n\t"
			"cbnz R5,_systick_dont_jump\n\t"
			"bl kernel_tick_counter\n\t"
			"bl kernel_schedule\n\t"
			"_systick_dont_jump:"
			: : :
	);

	kernel_handle_syscall();

	while (1)
		;
}

static void kernel_set_scheduler_freq(uint32_t hz)
{
	HWREG( NVIC_ST_CURRENT ) = 0;
	SysTickPeriodSet(SysCtlClockGet() / hz);
	SysTickIntRegister(svc_interrupt_handler);
	SysTickIntEnable();
	SysTickEnable();
}

void _exit(int status)
{
	sys_exit(0);
}
