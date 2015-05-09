/*
 * echo.c
 *
 *  Created on: Apr 28, 2015
 *      Author: Kevin
 */

#include "echo.h"

int echo_main(char* argv[], int argc)
{
	int i = 1;
	bool escaped = false;
	bool nonewline = false;
	if (argc > 1)
	{
		while(argv[i][0] == '-' && i < argc)
		{
			int arglen;
			if((arglen = fast_strlen(argv[i])) > 1)
			{
				while(--arglen)
				{
					switch(argv[i][arglen])
					{
					case 'n':
						nonewline = true;
						break;
					case 'e':
						escaped = true;
						break;
					}
				}
			}
			i++;
		}

		while (i < argc)
		{
			if(escaped)
			{
				char curchr, placechr;
				bool escapeseq = false;
				while((curchr = *(argv[i])))
				{
					if(curchr == '\\')
					{
						if(escapeseq)
						{
							placechr = '\\';
							escapeseq = false;
						}
						else
						{
							escapeseq = true;
						}
					}
					else
					{
						if(escapeseq)
						{
							escapeseq = false;
							switch(curchr)
							{
							case 'n':
								placechr = '\n';
								break;
							case 't':
								placechr = '\t';
								break;
							case 'r':
								placechr = '\r';
								break;
							case 'a':
								placechr = '\a';
								break;
							default:
								placechr = curchr;
								break;
							}
						}
						else
						{
							placechr = curchr;
						}
						sys_write(1, (uint8_t*)&placechr, 1);
					}
					argv[i]++;
				}
			}
			else
				sys_write(1, (uint8_t*)argv[i], fast_strlen(argv[i]));
			i++;
			if(i < argc)
				sys_write(1, (uint8_t*)" ", 1);
		}
	}
	if(!nonewline)
		sys_write(1, (uint8_t*)"\r\n", 2);
    return 0;
}
