/*
 * fileio.c
 *
 *  Created on: Apr 16, 2015
 *      Author: Kevin
 */

#include "fileio.h"

int write_main(char* argv[], int argc)
{
	fd_t fd;
	if(argc == 3)
	{
		bool succ;
		fd = (fd_t)fast_sntoul(argv[1], fast_strlen(argv[1]), 10, &succ);
		if(succ)
		{
			goto __write_run;
		}
	}

	sys_puts("Invalid arguments.\r\n", 100);
	sys_exit(-1);

	__write_run:

	return sys_write(fd, (uint8_t*)argv[2], fast_strlen(argv[2]));
}

int read_main(char* argv[], int argc)
{
	fd_t fd;
	size_t rdsiz;
	long ret;
	uint8_t strbuf[64];
	if (argc == 3)
	{
		bool succ;
		fd = (fd_t) fast_sntoul(argv[1], fast_strlen(argv[1]), 10, &succ);
		if (!succ)
		{
			goto __read_fail;
		}
		rdsiz = (fd_t) fast_sntoul(argv[2], fast_strlen(argv[2]), 10, &succ);
		if (succ)
		{
			if (rdsiz > 64)
			{

				goto __read_fail_too_many_bytes;
			}
			goto __read_run;
		}
	}

	__read_fail_too_many_bytes:

	sys_puts("Max read size is 64 bytes.\r\n", 100);

	__read_fail:

	sys_puts("Invalid arguments.\r\n", 100);
	sys_exit(-1);

	__read_run:

	ret = sys_read(fd, strbuf, rdsiz);

	if(ret == RW_INVALID)
	{
		sys_puts("Failed to read.\r\n", 100);
		sys_exit(-1);
	}

	//Serial_writebuf(UART_DEBUG_MODULE, strbuf, ret);

	long i;
	for(i = 0; i < ret; i++)
	{
		if(' ' <= strbuf[i] && strbuf[i] <= 127)
			sys_putc(strbuf[i]);
		else
			sys_putc(' ');
	}
	sys_puts("\r\n", 2);

	return ret;
}

int ioctl_main(char* argv[], int argc)
{
	sys_reset();
}

