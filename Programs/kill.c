/*
 * kill.c
 *
 *  Created on: Apr 29, 2015
 *      Author: Kevin
 */

#include "kill.h"

int kill_main(char* argv[], int argc)
{
	if (argc < 2)
		return -1;

	bool succ;
	tid_t killtid = fast_sntol(argv[1], fast_strlen(argv[1]), 10, &succ);
	if (!succ)
		return -1;

	succ = sys_kill(killtid);

	if(!succ)
		sys_puts("No such thread!\r\n", 100);

	return succ;
}
