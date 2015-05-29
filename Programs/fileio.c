/*
 * fileio.c
 *
 *  Created on: Apr 16, 2015
 *      Author: Kevin
 */

#include "fileio.h"

int cp_main(char* argv[], int argc)
{
	fd_t srcfd, destfd;
	uint8_t copybuf[64];
	if(argc == 3)
	{
		if(((srcfd = sys_open(argv[1], FMODE_R, FFLAG_NOBLOCK)) != FD_INVALID) &&
			((destfd = sys_open(argv[2], FMODE_W, FFLAG_CREAT)) != FD_INVALID))
		{
			int32_t copysiz;

			while((copysiz = sys_read(srcfd, copybuf, 64)) > 0)
			{
				int32_t copybufindex = 0;
				do
				{
					copybufindex += sys_write(
							destfd,
							copybuf + copybufindex,
							copysiz - copybufindex);
				} while (copybufindex < copysiz);
			}
		}
	}

	return -1;
}

int wc_main(char* argv[], int argc)
{
	fd_t srcfd;

	uint32_t charcnt = 0, wordcnt = 0, linecnt = 0;
	bool countchars = false, countwords = false, countlines = false;
	bool countselective = false;
	uint8_t readbuf[64];

	process_flags:
	argv++;
	argc--;
	if(!argc)
		return -1;
	if(fast_strcmp(argv[0], '-c') == 0)
	{
			countchars = true;
			countselective = true;
			goto process_flags;
	}
	else if(fast_strcmp(argv[0], '-w') == 0)
	{
			countchars = true;
			countselective = true;
			goto process_flags;
	}
	else if(fast_strcmp(argv[0], '-l') == 0)
	{
			countchars = true;
			countselective = true;
			goto process_flags;
	}

	int32_t readsiz;

//	while (sys_rem(STDIN) && ((readsiz = sys_read(STDIN, readbuf, 64)) > 0))
//	{
//		charcnt += readsiz;
//		while (readsiz--)
//		{
//			if (readbuf[readsiz] == ' ')
//				wordcnt++;
//			if (readbuf[readsiz] == '\n')
//				linecnt++;
//		}
//	}

	while (argc)
	{
		if ((srcfd = sys_open(argv[0], FMODE_R, FFLAG_NOBLOCK)) != FD_INVALID)
		{
			int32_t readsiz;

			while ((readsiz = sys_read(srcfd, readbuf, 64)) > 0)
			{
				charcnt += readsiz;
				while (readsiz--)
				{
					if (readbuf[readsiz] == ' ')
						wordcnt++;
					if (readbuf[readsiz] == '\n')
						linecnt++;
				}
			}

			sys_close(srcfd);
		}

		argv++;
		argc--;
	}

	if(!countselective)
	{
		countchars = countwords = countlines = true;
	}
	char* printbuf = (char*) readbuf;
	if (countchars)
	{
		fast_snprintf(printbuf, 64, "c: %ud ", charcnt);
		s_puts(printbuf);
	}
	if (countwords)
	{
		fast_snprintf(printbuf, 64, "w: %ud ", wordcnt);
		s_puts(printbuf);
	}
	if (countlines)
	{
		fast_snprintf(printbuf, 64, "l: %ud ", linecnt);
		s_puts(printbuf);
	}
	s_puts("\r\n");

	return -1;
}

int mv_main(char* argv[], int argc)
{
	return -1;
}

int cat_main(char* argv[], int argc)
{
	char printbuf[64];
	int32_t readsiz;
	if (argc >= 2)
	{
		uint32_t findex = 1;
		while(findex < argc)
		{
			if (fast_strcmp(argv[findex], "-") == 0)
			{
				// TODO: Reader should be awoken by closing of pipe
				while (sys_rem(STDIN) && ((readsiz = sys_read(STDIN, (uint8_t*) printbuf, 64))
								!= RW_INVALID))
				{
					if (readsiz == 0)
						break;

					sys_write(STDOUT, (uint8_t*) printbuf, readsiz);
				}
			}
			else
			{
				fd_t fd;
				if ((fd = sys_open(argv[findex], FMODE_R, 0)) == FD_INVALID)
				{
					s_puts("Failed to open \'");
					s_puts(argv[findex]);
					s_puts("\'\r\n");
					return -1;
				}

				// TODO: This read shouldn't block if there are no writers; because
				// reader/writer tracking hasn't been implemented yet, this has
				// to check for the remaining characters in the file
				while (sys_rem(fd)
						&& ((readsiz = sys_read(fd, (uint8_t*) printbuf, 64))
								!= RW_INVALID))
				{
					if (readsiz == 0)
						break;

					sys_write(STDOUT, (uint8_t*) printbuf, readsiz);
				}

				sys_close(fd);
			}

			findex++;
		}
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

