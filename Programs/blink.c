/*
 * blink.c
 *
 *  Created on: Apr 29, 2015
 *      Author: Kevin
 */

#include "blink.h"

#include "inc/hw_memmap.h"

int blink_main(char* argv[], int argc)
{
	if(argc < 2)
		return -1;

	bool succ;
	uint8_t led_number = fast_sntoul(argv[1], fast_strlen(argv[1]), 10, &succ);
	if(!succ)
		return -1;

	led_number = 2 << led_number;
	sys_set_port_dirs(GPIO_PORTF_BASE, led_number);
	while (1)
	{
		sys_write_port(GPIO_PORTF_BASE, led_number, led_number);
		sys_sleep(500);
		sys_write_port(GPIO_PORTF_BASE, led_number, 0);
		sys_sleep(500);
	}
}
