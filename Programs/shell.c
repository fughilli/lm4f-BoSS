/*
 * shell.c
 *
 *  Created on: Mar 28, 2015
 *      Author: Kevin
 */

#include "shell.h"
#include <stdint.h>
#include "../debug_serial.h"

const uint8_t _bsp_seq[] = {SHELL_BACKSPACE, ' ', SHELL_BACKSPACE};

#define _SHELL_PROMPT_LABEL_ "DankOS"

#define SHELL_IFS (' ')

char shell_lineBuffer[SHELL_LINE_BUFFER_SIZE];
uint32_t shell_lineBufferIndex;

char* shell_argBuffer[SHELL_MAX_ARGS];

const char shell_error_too_many_args[] = "Parse error: too many arguments!";
const char shell_error_unknown_command[] = "Error: unknown command \'";
const char shell_error_mismatched_quotes[] = "Error: mismatched quotes!";

void shell_processLine(void);

shell_progMapEntry_t shell_progMap[SHELL_MAX_PROGRAMS];

void shell_initProgMap()
{
    int i;
    for(i = 0; i < SHELL_MAX_PROGRAMS; i++)
    {
        shell_progMap[i].name[0] = 0;
    }
}

bool shell_registerProgram(const char* name, shell_func_t prog_main)
{
    if(fast_strlen(name) > SHELL_MAX_PROGRAM_NAMELEN-1)
        return false;

    int i;
    for(i = 0; i < SHELL_MAX_PROGRAMS; i++)
    {
        if(shell_progMap[i].name[0] == 0)
        {
            fast_strcpy(shell_progMap[i].name, name);
            shell_progMap[i].prog_main = prog_main;
            return true;
        }
    }

    return false;
}

shell_func_t shell_getProg(const char* name)
{
    if(fast_strlen(name) > SHELL_MAX_PROGRAM_NAMELEN-1)
        return NULL;

    int i;
    for(i = 0; i < SHELL_MAX_PROGRAMS; i++)
    {
        if(fast_strcmp(shell_progMap[i].name, name) == 0)
        {
            return shell_progMap[i].prog_main;
        }
    }

    return NULL;
}

void shell_main(void* arg)
{
    while (1)
    {
        Serial_puts(UART_DEBUG_MODULE, _SHELL_PROMPT_LABEL_ ":>", 100);
        shell_lineBufferIndex = 0;

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
                if (shell_lineBufferIndex > 0)
                {
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
                continue;
            }
            else
            {
                shell_lineBuffer[shell_lineBufferIndex++] = nextChar;
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
	shell_run_shim_params_t* shimparams = ((shell_run_shim_params_t*)arg);
	int ret = shimparams->func(shimparams->argv, shimparams->argc);

	sys_unlock(&shim_print_lock);

	sys_exit(ret);
}

void shell_processLine(void)
{
    // Initial term is at start of line
    char* shell_startOfTerm = shell_lineBuffer;

    char quoteChar = 0;
    char lastChar = 0;

    // Find end of term (delimited on IFS)
    uint32_t shell_lineIndex, shell_argIndex = 0;
    for (shell_lineIndex = 0; shell_lineIndex < shell_lineBufferIndex;
            shell_lineIndex++)
    {
    	char nextChar = shell_lineBuffer[shell_lineIndex];

    	if(quoteChar)
    	{
    		bool breakOnQuote = false;
    		for (; shell_lineIndex < shell_lineBufferIndex;
    		            shell_lineIndex++)
    		    {
    				nextChar = shell_lineBuffer[shell_lineIndex];
    				if(nextChar == quoteChar)
    				{
    					breakOnQuote = true;
    					break;
    				}
    		    }

    		if(!breakOnQuote)
    		{
    			Serial_puts(UART_DEBUG_MODULE, shell_error_mismatched_quotes, 100);
    			return;
    		}
    	}

		switch (nextChar)
		{
		case '\'':
		case '\"':
			if(!quoteChar)
			{
				quoteChar = nextChar;
				shell_startOfTerm = &shell_lineBuffer[shell_lineIndex + 1];
				break;
			}
			else if(nextChar == quoteChar)
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
			while(shell_lineBuffer[shell_lineIndex + 1] == SHELL_IFS && shell_lineIndex != shell_lineBufferIndex)
				shell_lineIndex++;
			shell_startOfTerm = &shell_lineBuffer[shell_lineIndex + 1];
			break;
		}
    }

    shell_run_shim_params_t shim_params;
    if((shim_params.func = shell_getProg(shell_argBuffer[0])))
    {
    	shim_params.argc = shell_argIndex;
    	shim_params.argv = shell_argBuffer;

    	while(!sys_lock(&shim_print_lock))
    		sys_sleep(10);

    	sys_spawn(shell_run_shim, &shim_params);

    	while(shim_print_lock)
    		sys_sleep(10);
    }
    else
    {
    	Serial_puts(UART_DEBUG_MODULE, shell_error_unknown_command, fast_strlen(shell_error_unknown_command));
        Serial_puts(UART_DEBUG_MODULE, shell_argBuffer[0], fast_strlen(shell_argBuffer[0]));
        Serial_puts(UART_DEBUG_MODULE, "\'\r\n", 3);
    }
}

