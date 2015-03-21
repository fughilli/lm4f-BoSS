/*
 * main.c
 */
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "inc/hw_uart.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include <string.h>
#include <stdio.h>
#include "syscalls.h"
#include "debug_serial.h"
#include "kernel.h"
#include "thread.h"

lock_t printlock = LOCK_UNLOCKED;

// The main routine for our application
void app_main2(void* arg)
{
	// Loop counter
	uint8_t counter = (uint32_t)arg;

	while(1)
	{
		// Increment the loop counter
		counter++;
		if(counter == 10)
			counter = 0;

		// Lock to write to the console
		while(!sys_lock(&printlock))
			;

		// Print out a message!
		sys_puts("Thread #", 100);
		sys_putc('0' + sys_get_tid());
		sys_putc('0' + counter);
		sys_puts(" running\r\n", 100);

		// Unlock when done; the kernel calls
		//  kernel_schedule() after a sys_unlock()
		//  to ensure fairness
		sys_unlock(&printlock);
	}
}

// Start two instances of the app_main2 application;
//  start the first instance's loop counter at 5,
//  and the second at 0
void app_main(void* arg)
{
    if(sys_fork())
    {
        thread_spawn(app_main2, (void*)0);
    }
    else
    {
        thread_spawn(app_main2, (void*)5);
    }

	sys_exit(0);
}

// Program entry point; initializes kernel
//  and then spawns thread 1 (app_main)
int main(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

	Serial_init();

	kernel_init();

	thread_spawn(app_main, 0);

	sys_exit(0);
}


