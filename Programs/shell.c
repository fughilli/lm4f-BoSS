/*
 * shell.c
 *
 *  Created on: Mar 28, 2015
 *      Author: Kevin
 */

#include "shell.h"
#include <stdint.h>
#include "../debug_serial.h"

const uint8_t _bsp_seq[] =
{ SHELL_BACKSPACE, ' ', SHELL_BACKSPACE };

#define _SHELL_PROMPT_LABEL_ "DankOS"

#define SHELL_IFS (' ')
#define SHELL_ESC_CHAR ('\x1B')
#define SHELL_LARROW_SEQ ("\x1B[D")
#define SHELL_RARROW_SEQ ("\x1B[C")
#define SHELL_UARROW_SEQ ("\x1B[A")
#define SHELL_DARROW_SEQ ("\x1B[B")

char shell_lineBuffer[SHELL_LINE_BUFFER_SIZE];
uint32_t shell_lineBufferIndex;
uint32_t shell_lineBufferInsertIndex;

char* shell_argBuffer[SHELL_MAX_ARGS];
uint32_t shell_lineIndex, shell_argIndex;
int lastret;

char shell_varBuf[16];

const char shell_error_too_many_args[] = "Parse error: too many arguments!";
const char shell_error_unknown_command[] = "Error: unknown command \'";
const char shell_error_mismatched_quotes[] = "Error: mismatched quotes!";

void shell_processLine(void);

int shell_jobs_main(char* argv[], int argc);

shell_progMapEntry_t shell_progMap[SHELL_MAX_PROGRAMS];

int shell_help_main(char* argv[], int argc)
{
	Serial_puts(UART_DEBUG_MODULE, "The following programs are available:\r\n", 100);
	int i;
	for(i = 0; i < SHELL_MAX_PROGRAMS; i++)
	{
		if(fast_strlen(shell_progMap[i].name))
		{
			Serial_puts(UART_DEBUG_MODULE, shell_progMap[i].name, 100);
			Serial_puts(UART_DEBUG_MODULE, "\r\n", 2);
		}
	}
	return 0;
}

void shell_initProgMap()
{
	lastret = 0;
	int i;
	for (i = 0; i < SHELL_MAX_PROGRAMS; i++)
	{
		shell_progMap[i].name[0] = 0;
	}

	shell_registerProgram("help", shell_help_main);
	shell_registerProgram("jobs", shell_jobs_main);
}

bool shell_registerProgram(const char* name, shell_func_t prog_main)
{
	if (fast_strlen(name) > (SHELL_MAX_PROGRAM_NAMELEN - 1))
		return false;

	int i;
	for (i = 0; i < SHELL_MAX_PROGRAMS; i++)
	{
		if (shell_progMap[i].name[0] == 0)
		{
			fast_strcpy(shell_progMap[i].name, name);
			shell_progMap[i].prog_main = prog_main;
			return true;
		}
	}

	return false;
}

int shell_getProgIdx(const char* name)
{
	if (fast_strlen(name) > SHELL_MAX_PROGRAM_NAMELEN - 1)
		return -1;

	int i;
	for (i = 0; i < SHELL_MAX_PROGRAMS; i++)
	{
		if (fast_strcmp(shell_progMap[i].name, name) == 0)
		{
			return i;
		}
	}

	return -1;
}

shell_func_t shell_getProg(const char* name)
{
	int i = shell_getProgIdx(name);
	if(i > 0)
		return shell_progMap[i].prog_main;

	return NULL;
}

void shell_main(void* arg)
{
	while (1)
	{
		Serial_puts(UART_DEBUG_MODULE, _SHELL_PROMPT_LABEL_ ":>", 100);
		shell_lineBufferIndex = 0;
		shell_lineBufferInsertIndex = 0;

		while (1)
		{
			while (!Serial_avail(UART_DEBUG_MODULE))
			{
				sys_sleep(10);
			}

			char nextChar = Serial_getc(UART_DEBUG_MODULE);

			// Handle newline (end-of-command)
			if (nextChar == '\r')
			{
				if (shell_lineBufferIndex)
				{
					shell_lineBuffer[shell_lineBufferIndex++] = 0;

					Serial_puts(UART_DEBUG_MODULE, "\r\n", 2);

					shell_processLine();
					break;
				}
				else
				{
					Serial_puts(UART_DEBUG_MODULE, "\r\n", 2);
					break;
				}
			}
			else if (nextChar == SHELL_BACKSPACE)
			{
				if (shell_lineBufferInsertIndex > 0)
				{
					fast_memmove(
							&shell_lineBuffer[shell_lineBufferInsertIndex - 1],
							&shell_lineBuffer[shell_lineBufferInsertIndex],
							shell_lineBufferIndex
									- shell_lineBufferInsertIndex);
					shell_lineBufferInsertIndex--;
					shell_lineBufferIndex--;
					Serial_writebuf(UART_DEBUG_MODULE, _bsp_seq, 3);
				}
				else
				{
					Serial_putc(UART_DEBUG_MODULE, SHELL_BELL_CHAR);
				}
				continue;
			}
			// Handle special character
			else if (nextChar < ' ')
			{
				if (nextChar == SHELL_ESC_CHAR)
				{
					if (Serial_getc(UART_DEBUG_MODULE) == '[')
					{
						char control_char;
						switch (control_char = Serial_getc(UART_DEBUG_MODULE))
						{
//            			case 'A':
//            				continue;
//            				break;
//            			case 'B':
//            				continue;
//            				break;
						case '4':
							Serial_getc(UART_DEBUG_MODULE); // Consume '~'
							shell_lineBufferInsertIndex = shell_lineBufferIndex;
							Serial_writebuf(UART_DEBUG_MODULE, "\x1B[4~", 4);
							continue;
							break;
						case '1':
							Serial_getc(UART_DEBUG_MODULE); // Consume '~'
							shell_lineBufferInsertIndex = 0;
							Serial_writebuf(UART_DEBUG_MODULE, "\x1B[1~", 4);
							continue;
							break;
						case 'D':
							if (shell_lineBufferInsertIndex > 0)
							{
								shell_lineBufferInsertIndex--;
								Serial_writebuf(UART_DEBUG_MODULE, SHELL_LARROW_SEQ, 3);
							}
							else
							{
								Serial_putc(UART_DEBUG_MODULE, SHELL_BELL_CHAR);
							}
							continue;
							break;
						case 'C':
							if (shell_lineBufferInsertIndex
									< shell_lineBufferIndex)
							{
								shell_lineBufferInsertIndex++;
								Serial_writebuf(UART_DEBUG_MODULE, SHELL_RARROW_SEQ, 3);
							}
							else
							{
								Serial_putc(UART_DEBUG_MODULE, SHELL_BELL_CHAR);
							}
							continue;
							break;
						default:
							continue;
						}
					}
				}
				continue;
			}
			else
			{
				if (shell_lineBufferInsertIndex == shell_lineBufferIndex)
				{
					shell_lineBuffer[shell_lineBufferIndex++] = nextChar;
					shell_lineBufferInsertIndex++;
				}
				else if (shell_lineBufferInsertIndex < shell_lineBufferIndex)
				{
					fast_memmove(
							&shell_lineBuffer[shell_lineBufferInsertIndex + 1],
							&shell_lineBuffer[shell_lineBufferInsertIndex],
							shell_lineBufferIndex
									- shell_lineBufferInsertIndex);
					shell_lineBuffer[shell_lineBufferInsertIndex++] = nextChar;
					shell_lineBufferIndex++;
				}
			}

			// Handle line buffer overrun
			if (shell_lineBufferIndex == SHELL_LINE_BUFFER_SIZE)
			{
				Serial_putc(UART_DEBUG_MODULE, SHELL_BELL_CHAR);
				shell_lineBufferIndex--;
				continue;
			}

			Serial_putc(UART_DEBUG_MODULE, nextChar);
		}
	}
}

typedef struct
{
	shell_func_t func;
	char** argv;
	int argc;
} shell_run_shim_params_t;

lock_t shim_print_lock = LOCK_UNLOCKED;

void shell_run_shim(void* arg)
{
	shell_run_shim_params_t* shimparams = ((shell_run_shim_params_t*) arg);
	int ret = shimparams->func(shimparams->argv, shimparams->argc);

	sys_unlock(&shim_print_lock);

	sys_exit(ret);
}

typedef struct
{
	tid_t thread;
	const char* program;
} shell_job_t;

#define SHELL_MAX_JOBS (4)

shell_job_t shell_jobs[SHELL_MAX_JOBS];

#define SHELL_JOB_VALID(_job_) (0 <= ((_job_) - shell_jobs) && ((_job_) - shell_jobs) < SHELL_MAX_JOBS && (_job_)->program != NULL)

void shell_initJobs()
{
	int i;
	for(i = 0; i < SHELL_MAX_JOBS; i++)
	{
		shell_jobs[i].program = NULL;
	}
}

shell_job_t* shell_allocJob()
{
	int i;
	for(i = 0; i < SHELL_MAX_JOBS; i++)
	{
		if(shell_jobs[i].program == NULL)
			return &shell_jobs[i];
	}

	return NULL;
}

void shell_freeJob(shell_job_t* job)
{
	if(!SHELL_JOB_VALID(job))
		return;

	job->program = NULL;
}

void shell_job_waiter(void* arg)
{
	shell_job_t* job = (shell_job_t*)arg;
	if(!SHELL_JOB_VALID(job))
		sys_exit(-1);

	int retval = sys_wait(job->thread);

	char printbuf[32];
	fast_snprintf(printbuf, 32, "[%d] done %d\t\t\'", (int)(job - shell_jobs), retval);
	sys_puts(printbuf, 32);
	sys_puts(job->program, 100);
	sys_puts("\'\r\n", 3);

	shell_freeJob(job);
	sys_exit(0);
}

int shell_jobs_main(char* argv[], int argc)
{
	char printbuf[32];
	int i;
	for(i = 0; i < SHELL_MAX_JOBS; i++)
	{
		if(shell_jobs[i].program)
		{
			fast_snprintf(printbuf, 32, "[%d] running\t\t\'", i);
			sys_puts(printbuf, 32);
			sys_puts(shell_jobs[i].program, 100);
			sys_puts("\'\r\n", 3);
		}
	}

	return 0;
}

void shell_substitute_vars()
{
	fast_snprintf(shell_varBuf, 16, "%d", lastret);
	uint32_t vari;
	for(vari = 0; vari < shell_argIndex; vari++)
	{
		if(fast_strcmp(shell_argBuffer[vari], "$?") == 0)
		{
			shell_argBuffer[vari] = shell_varBuf;
		}
	}
}

void shell_processLine(void)
{
	// Initial term is at start of line
	char* shell_startOfTerm = shell_lineBuffer;

	char quoteChar = 0;

	bool background = false;

	// Find end of term (delimited on IFS)
	shell_argIndex = 0;
	for (shell_lineIndex = 0; shell_lineIndex < shell_lineBufferIndex;
			shell_lineIndex++)
	{
		char nextChar = shell_lineBuffer[shell_lineIndex];

		if (quoteChar)
		{
			bool breakOnQuote = false;
			for (; shell_lineIndex < shell_lineBufferIndex; shell_lineIndex++)
			{
				nextChar = shell_lineBuffer[shell_lineIndex];
				if (nextChar == quoteChar)
				{
					breakOnQuote = true;
					break;
				}
			}

			if (!breakOnQuote)
			{
				Serial_puts(UART_DEBUG_MODULE, shell_error_mismatched_quotes,
						100);
				return;
			}
		}

		switch (nextChar)
		{
		case '\'':
		case '\"':
			if (!quoteChar)
			{
				quoteChar = nextChar;
				shell_startOfTerm = &shell_lineBuffer[shell_lineIndex + 1];
				break;
			}
			else if (nextChar == quoteChar)
			{
				quoteChar = 0;
				shell_lineBuffer[shell_lineIndex] = 0;
				break;
			}
		case SHELL_IFS:
		case 0:
			// If the maximum argument count has been reached
			if (shell_argIndex == SHELL_MAX_ARGS)
			{
				Serial_puts(UART_DEBUG_MODULE, shell_error_too_many_args, 100);
				return;
			}

			// Swap the space character with a null byte
			shell_lineBuffer[shell_lineIndex] = 0;
			// Add the just-completed term to the argument list
			shell_argBuffer[shell_argIndex++] = shell_startOfTerm;
			// Start the next term after the null byte
			while (shell_lineBuffer[shell_lineIndex + 1] == SHELL_IFS
					&& shell_lineIndex != shell_lineBufferIndex)
				shell_lineIndex++;
			shell_startOfTerm = &shell_lineBuffer[shell_lineIndex + 1];
			break;
		}
	}

	shell_run_shim_params_t shim_params;
	int progidx = shell_getProgIdx(shell_argBuffer[0]);
	if (progidx > 0)
	{
		if(fast_strcmp(shell_argBuffer[shell_argIndex - 1], "&") == 0)
		{
			background = true;
			shell_argIndex--;
		}

		shim_params.func = shell_progMap[progidx].prog_main;
		shim_params.argc = shell_argIndex;
		shim_params.argv = shell_argBuffer;

		shell_substitute_vars();

		tid_t tid = sys_spawn(shell_run_shim, &shim_params);

		//TODO: Fix max jobs problem; handle job == NULL properly

		if(!background)
		{
			lastret = sys_wait(tid);
		}
		else
		{
			background = false;
			shell_job_t* job = shell_allocJob();
			if (job != NULL)
			{
				job->thread = tid;
				job->program = shell_progMap[progidx].name;
				sys_spawn(shell_job_waiter, job);
				char printbuf[32];
				fast_snprintf(printbuf, 32, "[%d] %d\r\n", (int)(job - shell_jobs),
						(int)tid);
				sys_puts(printbuf, 32);
			}
			else
			{
				sys_puts("Exceeded max jobs!\r\n", 100);
			}
		}

	}
	else
	{
		Serial_puts(UART_DEBUG_MODULE, shell_error_unknown_command,
				fast_strlen(shell_error_unknown_command));
		Serial_puts(UART_DEBUG_MODULE, shell_argBuffer[0],
				fast_strlen(shell_argBuffer[0]));
		Serial_puts(UART_DEBUG_MODULE, "\'\r\n", 3);
	}
}

