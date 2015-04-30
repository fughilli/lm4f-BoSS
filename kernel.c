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
#include "inc/hw_gpio.h"
#include "fast_utils.h"
#include "driverlib/mpu.h"
#include "driverlib/gpio.h"
#include "file.h"
#include "fsystem.h"
#include "os_config.h"

#include <stdbool.h>

#define _SECTION_DECLARE(_typedecl_,_symbol_,_arrs_,_val_,_section_) _typedecl_ (_symbol_) _arrs_ __attribute__((section("#_section"))) = (_val_)
#define _SECTION_STRING(_symbol_,_string_,_section_) _SECTION_DECLARE(const char,_symbol_,[],_string_,_section_)
#define _FLASH_STRING(_symbol_,_string_) _SECTION_STRING(_symbol_,_string_,FLASH)

//uint8_t kernel_stack[128] __attribute((aligned(8)));

fd_assoc_t file_table[MAX_FILES] __attribute__((section("SRAM")));

_FLASH_STRING(__k_thread_valid_incorrect_behavior, "thread_valid: incorrect behavior");
_FLASH_STRING(__k_thread_pos_incorrect_behavior, "thread_pos: incorrect behavior");
_FLASH_STRING(__k_kp_str_hdr, "Kernel panic: ");
_FLASH_STRING(__k_kp_str_hardfault, "hard fault");
_FLASH_STRING(__k_kp_str_nmi, "non-maskable interrupt");
_FLASH_STRING(__k_kp_str_default, "unimplemented ISR");
_FLASH_STRING(__k_kp_str_nl, "\r\n");
_FLASH_STRING(__k_r_str, "Going down for soft reset NOW!");
_FLASH_STRING(__k_kp_str_regtrace, "Registers before fault:\r\n");

const char __k_kp_str_regnames[THREAD_SAVED_REGISTERS_NUM][4] __attribute__((section("FLASH"))) =
{
		"SP ", "R0 ", "R1 ", "R2 ",
		"R3 ", "R4 ", "R5 ", "R6 ",
		"R7 ", "R8 ", "R9 ", "R10",
		"R11", "R12", "LR ", "PC ",
		"PSR"
};

const uint8_t __k_reg_sp_offsets[THREAD_SAVED_REGISTERS_NUM] __attribute__((section("FLASH"))) =
{
		0, 11, 12, 13,
		14, 0, 1, 2,
		9, 3, 4, 5,
		6, 15, 16, 17,
		18
};

tsleep_t systime_ms;
tsleep_t next_to_run_ms;

void kernel_schedule();
static void kernel_set_scheduler_freq(uint32_t hz);
void kernel_assert(bool cond, const char* fstr, uint32_t fstrlen);
void kernel_enable_mpu();
void kernel_set_memory_region(thread_t* thread);
void kernel_set_memory_region_flash();

void kernel_init()
{
	thread_init();

	ftable_init();

	thread_table[0].state = T_RUNNABLE;
	thread_table[0].id = 0;
	thread_current = &thread_table[0];

	systime_ms = 0;
	next_to_run_ms = UINT32_MAX;

	int i;
	for(i = 0; i < MAX_THREADS; i++)
	{
		kernel_assert(thread_pos(&thread_table[i]) == i,
				__k_thread_pos_incorrect_behavior, 100);
		kernel_assert(thread_valid(&thread_table[i]),
				__k_thread_valid_incorrect_behavior, 100);
	}

	kernel_set_scheduler_freq(KERNEL_SCHEDULER_IRQ_FREQ);

#if MEMORY_PROTECTION
	SysCtlPeripheralEnable(SYSCTL_PERIPH_MPU);

	kernel_set_memory_region_flash();
	// Have to set flash first, otherwise setting thread mode to unprivileged will result in a hard fault
	kernel_set_memory_region(thread_current);

	kernel_enable_mpu();
#endif
}

void kernel_enable_mpu()
{
	MPUEnable(MPU_CONFIG_PRIV_DEFAULT);

	//Set thread mode to unprivileged access
	asm volatile (
			"mrs R8,CONTROL\n\t"
			"and R8,R8,$0xFE\n\t"
			"add R8,R8,$0x01\n\t"
			"msr CONTROL,R8"
			);
}

static uint8_t bitpos(uint32_t mask)
{
	uint8_t ret;
	while(mask)
	{
		mask >>= 1;
		ret++;
	}
	return ret;
}

/**
 * Setup MPU region 0 to the bounds of the thread stack
 */
#define THREAD_MEM_SIZE_MASK ((LOG2_THREAD_MEM_SIZE - 1) << 1)
void kernel_set_memory_region(thread_t* thread)
{
	//MPURegionDisable(0);
	MPURegionSet(0, (uint32_t)thread_mem[thread_pos(thread)], THREAD_MEM_SIZE_MASK | MPU_RGN_PERM_PRV_RW_USR_RW | MPU_RGN_PERM_NOEXEC);
	MPURegionEnable(0);
}

/**
 * Setup MPU region 1 to the bounds of the system flash
 */
void kernel_set_memory_region_flash()
{
	//MPURegionDisable(1);
	MPURegionSet(1, 0, MPU_RGN_PERM_EXEC | MPU_RGN_PERM_PRV_RO_USR_RO | MPU_RGN_SIZE_256K);
	MPURegionEnable(1);
}

__attribute__((noreturn))
void kernel_panic(const char* kpstr, uint32_t kpstrlen)
{
	Serial_puts(UART_DEBUG_MODULE, __k_kp_str_hdr, sizeof(__k_kp_str_hdr));
	Serial_puts(UART_DEBUG_MODULE, kpstr, kpstrlen);
	Serial_puts(UART_DEBUG_MODULE, __k_kp_str_nl, sizeof(__k_kp_str_nl));

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

void kernel_close_fds(thread_t* thread)
{
	if(!thread)
		return;

	int i;
	for(i = 0; i < THREAD_MAX_OPEN_FDS; i++)
	{
		close(thread->open_fds[i]);
	}
}

__attribute__((noreturn))
void kernel_run(thread_t* thread)
{
	thread_current = thread;

	kernel_set_memory_region(thread_current);

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
			"dsb\n\t"
			"isb\n\t"
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
		thread_notify_waiting(thread_current);
		kernel_close_fds(thread_current);
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

	case SYSCALL_FLUBBER:
		Serial_puts(UART_DEBUG_MODULE, "Flubber!", 8);
		kernel_run(thread_current);
		break;

	case SYSCALL_PUTC:
		// call low-level IO send function
		// argument is thread_current->regs.R1
		Serial_putc(UART_DEBUG_MODULE, thread_current->regs.R1);
		kernel_run(thread_current);
		break;

	case SYSCALL_PUTS:
		// TODO: VULNERABILITY
		Serial_puts(UART_DEBUG_MODULE, (char*) thread_current->regs.R1, thread_current->regs.R2);
		kernel_run(thread_current);
		break;

	case SYSCALL_LOCK:
		// If the lock is already taken, return 0 and resume the spinlock
		// TODO: VULNERABILITY check for existence inside program memory bounds;
		// this can be used to set arbitrary memory locations in privileged mode
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

			if((thread_current->scnt - systime_ms) < (next_to_run_ms - systime_ms))
				next_to_run_ms = thread_current->scnt;

			thread_current->state = T_SLEEPING;
			kernel_schedule();
		}
		else
		{
			thread_current->regs.R0 = 0;
			kernel_run(thread_current);
		}
		break;

	case SYSCALL_KILL:
		// TODO: VULNERABILITY
		child_thread = tt_entry_for_tid((tid_t)thread_current->regs.R1);
		if(child_thread)
		{
			thread_notify_waiting(child_thread);
			kernel_close_fds(thread_current);
			thread_current->regs.R0 = thread_kill(child_thread);
		}
		else
		{
			thread_current->regs.R0 = 0;
		}
		kernel_run(thread_current);

	case SYSCALL_RESET:
		// TODO: VULNERABILITY
		// Print a reset message and then initiate a software reset
		Serial_puts(UART_DEBUG_MODULE, __k_r_str, fast_strlen(__k_r_str));
		Serial_puts(UART_DEBUG_MODULE, __k_kp_str_nl, fast_strlen(__k_kp_str_nl));
		Serial_flush(UART_DEBUG_MODULE);
		SysCtlReset();
		break;

	case SYSCALL_SET_PORT_DIRS:
		GPIOPinTypeGPIOOutput(thread_current->regs.R1, thread_current->regs.R2);
		//GPIOPinTypeGPIOInput(thread_current->regs.R1, ~thread_current->regs.R2);
		//HWREG(thread_current->regs.R1 + GPIO_O_DIR) = (HWREG(thread_current->regs.R1 + GPIO_O_DIR) & 0xFFFFFF00) | (thread_current->regs.R2 & 0xFF);
		//HWREG(thread_current->regs.R1 + GPIO_O_AFSEL) &= ~0xFF;
		kernel_run(thread_current);
		break;

	case SYSCALL_WRITE_PORT:
		GPIOPinWrite(thread_current->regs.R1, thread_current->regs.R2, thread_current->regs.R3);
		kernel_run(thread_current);
		break;

	case SYSCALL_READ_PORT:
		thread_current->regs.R0 = GPIOPinRead(thread_current->regs.R1, 0xFF);
		kernel_run(thread_current);
		break;

	case SYSCALL_READ:
		// TODO: VULNERABILITY
		thread_current->regs.R0 = (uint32_t) read(
				(fd_t) thread_current->regs.R1,
				(uint8_t*) thread_current->regs.R2,
				(int32_t) thread_current->regs.R3);
		kernel_schedule();
		break;

	case SYSCALL_WRITE:
		// TODO: VULNERABILITY
		thread_current->regs.R0 = (uint32_t) write(
				(fd_t) thread_current->regs.R1,
				(const uint8_t*) thread_current->regs.R2,
				(int32_t) thread_current->regs.R3);
		kernel_schedule();
		break;

	case SYSCALL_IOCTL:
		// TODO: VULNERABILITY
		thread_current->regs.R0 = (uint32_t) ioctl(
						(fd_t) thread_current->regs.R1,
						(uint32_t) thread_current->regs.R2,
						(void*) thread_current->regs.R3);
		kernel_schedule();
		break;

	case SYSCALL_SPAWN:
		// TODO: VULNERABILITY
		thread_current->regs.R0 = (uint32_t) thread_spawn(
				(void(*)(void*)) thread_current->regs.R1,
				(void*) thread_current->regs.R2);
		kernel_schedule();
		break;

	case SYSCALL_WAIT:
		thread_current->state = T_BLOCKED;
		thread_current->wait_func = WAITING_ON_THREAD;
		// The thread id that thread_current is waiting on
		// is stored in R1
		kernel_schedule();
		break;

	case SYSCALL_OPEN:
		thread_current->regs.R0 = open((const char*) thread_current->regs.R1,
				(fmode_t) thread_current->regs.R2,
				(fflags_t) thread_current->regs.R3);
		kernel_run(thread_current);
		break;

	case SYSCALL_CLOSE:
		close((fd_t) thread_current->regs.R1);
		kernel_run(thread_current);
		break;

	case SYSCALL_LISTDIR:
		thread_current->regs.R0 = listdir((char*) thread_current->regs.R1,
				(size_t) thread_current->regs.R2);
		kernel_run(thread_current);
		break;

	case SYSCALL_RWDIR:
		rwdir();
		kernel_run(thread_current);
		break;

	case SYSCALL_CHDIR:
		thread_current->regs.R0 = chdir((const char*)thread_current->regs.R1);
		kernel_run(thread_current);
		break;

	case SYSCALL_SEEK:
		thread_current->regs.R0 = seek((fd_t)thread_current->regs.R1,
				(int32_t)thread_current->regs.R2);
		kernel_run(thread_current);
		break;

	case SYSCALL_MKDIR:
			thread_current->regs.R0 = mkdir((const char*)thread_current->regs.R1);
			kernel_run(thread_current);
			break;

	case SYSCALL_UNLINK:
			thread_current->regs.R0 = unlink((const char*)thread_current->regs.R1);
			kernel_run(thread_current);
			break;

	}

	kernel_panic("unknown syscall", 15);

	while(1)
		;

}

static void kernel_tick_counter(void)
{
	systime_ms++;

	if(systime_ms != next_to_run_ms)
		return;

	next_to_run_ms = UINT32_MAX;

	int i;
	for (i = 0; i < MAX_THREADS; i++)
	{
		if (thread_table[i].state == T_SLEEPING)
		{
			// If this tick is the thread's wakeup time
			if (systime_ms == thread_table[i].scnt)
			{
				// Wake it up
				thread_table[i].state = T_RUNNABLE;
			}
			// If this is not the thread's wakeup time
			// sort wakeup times to determine next runtime
			else
			{
				// Normalize the time (modulo UINT32_MAX + 1)
				tsleep_t normtime = thread_table[i].scnt - systime_ms;
				if(normtime < next_to_run_ms)
				{
					next_to_run_ms = normtime;
				}
			}
		}
	}

	// Denormalize (relative to current systime_ms)
	next_to_run_ms += systime_ms;
}

// Called by SVC
__attribute__((noreturn))
void svc_interrupt_handler()
{
	// Push the context of the trapping thread
	asm volatile (
			//"dsb\n\t" <-- these should occur right before exceptional return
			//"isb\n\t" 	to make sure that modifications to memory are reflected
			//				before thread resume
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
