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

int open_main(char* argv[], int argc)
{
	char printbuf[32];
	fflags_t flags;
	fmode_t mode;
	if(argc == 4)
	{
		bool succ;
		mode = (fmode_t)fast_sntoul(argv[2], fast_strlen(argv[2]), 16, &succ);
		if (!succ)
		{
			goto __open_fail;
		}
		flags = (fflags_t)fast_sntoul(argv[3], fast_strlen(argv[3]), 16, &succ);
		if (!succ)
		{
			goto __open_fail;
		}
	}

	fd_t resfd = sys_open(argv[1], mode, flags);
	fast_snprintf(printbuf, 32, "fd: %d\r\n", (int)resfd);
	sys_puts(printbuf, 32);
	return 0;

	__open_fail:
	sys_puts("Invalid arguments.\r\n", 100);
	return -1;
}

int close_main(char* argv[], int argc)
{
	fd_t argfd;
	if(argc == 2)
	{
		bool succ;
		argfd = (fd_t)fast_sntoul(argv[1], fast_strlen(argv[1]), 10, &succ);
		if (!succ)
		{
			goto __close_fail;
		}
	}

	sys_close(argfd);
	sys_puts("Closed file.\r\n", 32);
	return 0;

	__close_fail:
	sys_puts("Invalid arguments.\r\n", 100);
	return -1;
}

int cat_main(char* argv[], int argc)
{
	char printbuf[64];
	int32_t readsiz;
	if (argc == 2)
	{
		fd_t fd;
		if ((fd = sys_open(argv[1], FMODE_R, 0)) == FD_INVALID)
		{
			s_puts("Failed to open \'");
			s_puts(argv[1]);
			s_puts("\'\r\n");
			return -1;
		}


		while((readsiz = sys_read(fd, (uint8_t*)printbuf, 64)) != RW_INVALID)
		{
			if(readsiz == 0)
				break;

			sys_write(STDOUT, printbuf, readsiz);
		}

		sys_close(fd);
		return 0;
	}

	s_puts("Invalid arguments.\r\n");
	return -1;
}

int ls_main(char* argv[], int argc)
{
	char fnamebuf[32];

	sys_rwdir();

	while(sys_listdir(fnamebuf, 32))
	{
		s_puts(fnamebuf);
		s_puts("\r\n");
	}

	return 0;
}

int cd_main(char* argv[], int argc)
{
	if(argc == 2)
	{
		if(sys_chdir(argv[1]))
			return 0;

		s_puts("Failed to chdir!\r\n");
		return -1;
	}

	s_puts("Invalid arguments.\r\n");
	return -1;
}

int rm_main(char* argv[], int argc)
{
	bool failed = false;
	if(argc > 1)
	{
		while(--argc)
		{
			if(!sys_unlink(argv[argc]))
			{
				failed = true;
				s_puts("Failed to remove \'");
				s_puts(argv[argc]);
				s_puts("\'\r\n");
			}
		}
	}

	if(failed)
		return -1;
	return 0;
}

int touch_main(char* argv[], int argc)
{
	bool failed = false;
	fd_t fd;
	if (argc > 1)
	{
		while (--argc)
		{
			if ((fd = sys_open(argv[argc], FMODE_W, FFLAG_CREAT)) == FD_INVALID)
			{
				failed = true;
				s_puts("Failed to create \'");
				s_puts(argv[argc]);
				s_puts("\'\r\n");
			}

			sys_close(fd);
		}
	}

	if (failed)
		return -1;
	return 0;
}

int mkdir_main(char* argv[], int argc)
{
	bool failed = false;
	if (argc > 1)
	{
		while (--argc)
		{
			if (!sys_mkdir(argv[argc]))
			{
				failed = true;
				s_puts("Failed to create directory \'");
				s_puts(argv[argc]);
				s_puts("\'\r\n");
			}
		}
	}

	if (failed)
		return -1;
	return 0;
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

	s_puts("Max read size is 64 bytes.\r\n");

	__read_fail:

	s_puts("Invalid arguments.\r\n");
	sys_exit(-1);

	__read_run:

	ret = sys_read(fd, strbuf, rdsiz);

	if(ret == RW_INVALID)
	{
		s_puts("Failed to read.\r\n");
		sys_exit(-1);
	}

	//Serial_writebuf(UART_DEBUG_MODULE, strbuf, ret);

	long i;
	for(i = 0; i < ret; i++)
	{
		if(' ' <= strbuf[i] && strbuf[i] <= 127)
			s_putc(strbuf[i]);
		else
			s_putc(' ');
	}
	s_puts("\r\n");

	return ret;
}

int ioctl_main(char* argv[], int argc)
{
	sys_reset();
}

