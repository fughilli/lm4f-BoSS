/*
 * main.c
 */

//#include "kernel.h"
//#include "thread.h"
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

void app_main2(void* arg)
{
	char pbuf[32];
	sprintf(pbuf, "tid: %d\r\n", sys_get_tid());
	Serial_puts(pbuf, 100);
	while(1)
	{
		Serial_puts((char*)arg, 100);
		sys_yield();
	}
}

void app_main(void* arg)
{
	tid_t tid = sys_get_tid();
	thread_spawn(app_main2, "alice!\r\n");
	thread_spawn(app_main2, "bob!\r\n");

	while(1)
		tid = sys_get_tid();
		//sys_yield();

	sys_exit(0);
}

int main(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

	Serial_init();

	kernel_init(app_main);

	while(1)
		;

	return 0;
}


