/*
 * memprint.c
 *
 *  Created on: Apr 16, 2015
 *      Author: Kevin
 */

#include "osprogram.h"
#include "memprint.h"

#include <stdint.h>
#include <stdbool.h>

/*
 * memmanip [-u] [-c|-s|-i|-l] <ADDR> <VAL> [<VAL> ...]
 *           ^^    ^  ^  ^  ^     ^     ^      ^^^^^
 *        unsigned |  |  |  |  address  |        |
 *               char |  |  |         value      |
 *                  short|  |             subsequent values
 *                      int |             set at the stride
 *                         long           of the datatype
 */
int memmanip_main(char* argv[], int argc)
{
	bool succ;
	uint32_t vals[2];
	char prstr[32];
	if(argc == 3)
	{

	}
}

int memprint_main(char* argv[], int argc)
{
	bool succ;
	uint32_t vals[2];
	char prstr[32];
	if (argc == 3)
	{
		int i;
		for (i = 1; i < 3; i++)
		{
			// Check if the value is in hex
			if (fast_memcmp(argv[i], "0x", 2) == 0)
			{
				argv[i] += 2;
				vals[i-1] = fast_sntoul(argv[i], fast_strlen(argv[i]), 16, &succ);
			}
			else
			{
				vals[i-1] = fast_sntoul(argv[i], fast_strlen(argv[i]), 10, &succ);
			}

			if (!succ)
			{
				fast_snprintf(prstr, 32, "Arg %d invalid.", i - 1);
				sys_puts(prstr, fast_strlen(prstr));
				return -1;
			}
		}

		uint32_t base = vals[0] - (vals[0] % 8);
		uint32_t end = base + fast_nextmulof(vals[1] + (vals[0] - base), 8);
		uint32_t iter;

		for (; base < end; base += 8)
		{
			uint8_t* memseg = (uint8_t*) base;
			fast_snprintf(prstr, 32, "0x%08x:\t", base);
			sys_puts(prstr, fast_strlen(prstr));

			// Print the hex
			for (iter = 0; iter < 8; iter++)
			{
				// Format the hex into prstr
				fast_snfmti(prstr, 32, memseg[iter], 16);

				// Print 0x__
				uint8_t hexsiz = fast_strlen(prstr);
				hexsiz = 2 - hexsiz;
				sys_puts(" 0x", 2);
				while (hexsiz--)
				{
					sys_putc('0');
				}
				sys_puts(prstr, fast_strlen(prstr));
			}

			// Put a tab
			sys_puts("\t\"", 2);

			for (iter = 0; iter < 8; iter++)
			{
				if (memseg[iter] >= ' ' && memseg[iter] <= 127)
				{
					sys_putc((char) memseg[iter]);
				}
				else
				{
					sys_putc(' ');
				}
			}

			sys_puts("\"\r\n", 3);
		}

		return 0;
	}

	sys_puts("Usage:\r\n\tmemprint <ADDR> <LEN>\r\n", 100);
	return 0;
}
