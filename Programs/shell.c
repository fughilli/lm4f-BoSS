/*
 * shell.c
 *
 *  Created on: Mar 28, 2015
 *      Author: Kevin
 */

#include "shell.h"
#include <stdint.h>
#include "cmdline_parse/cmdline_parse.h"
#include "../pipe.h"

const uint8_t _bsp_seq[] =
{ SHELL_BACKSPACE, ' ', SHELL_BACKSPACE };

#define SHELL_DEFAULT_PROMPT "BalkOS"

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

char shell_labelBuf[16];

char shell_varBuf[16];

const char shell_error_too_many_args[] = "Parse error: too many arguments!";
const char shell_error_unknown_command[] = "Error: unknown command \'";
const char shell_error_mismatched_quotes[] = "Error: mismatched quotes!";

void shell_processLine(void);

int shell_jobs_main(char* argv[], int argc);

shell_progMapEntry_t shell_progMap[SHELL_MAX_PROGRAMS];

int shell_help_main(char* argv[], int argc)
{
	s_puts("The following programs are available:\r\n");
	int i;
	for(i = 0; i < SHELL_MAX_PROGRAMS; i++)
	{
		if(fast_strlen(shell_progMap[i].name))
		{
			s_puts(shell_progMap[i].name);
			s_puts("\r\n");
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

uint8_t shell_readChar()
{
	uint8_t ret;
	sys_read(STDIN,&ret,1);
	return ret;
}

void shell_main(void* arg)
{
	fd_t labelfd = sys_open("/settings/shell/label.txt", FMODE_R, 0);
	if (labelfd == FD_INVALID)
	{
		fast_strcpy(shell_labelBuf, SHELL_DEFAULT_PROMPT);
	}
	else
	{
		int labellen = sys_read(labelfd, (uint8_t*)shell_labelBuf, 16);
		labellen = (labellen > 15) ? 15 : labellen;
		shell_labelBuf[labellen] = '\0';

		sys_close(labelfd);
	}

	while (1)
	{
		s_puts(shell_labelBuf);
		s_puts(":>");
		shell_lineBufferIndex = 0;
		shell_lineBufferInsertIndex = 0;

		while (1)
		{
			while (!sys_rem(STDIN))
			{
				sys_sleep(10);
			}

			char nextChar;
			sys_read(STDIN,&nextChar,1);

			// Handle newline (end-of-command)
			if (nextChar == '\r')
			{
				if (shell_lineBufferIndex)
				{
					shell_lineBuffer[shell_lineBufferIndex++] = 0;

					s_puts("\r\n");

					shell_processLine();
					break;
				}
				else
				{
					s_puts("\r\n");
					break;
				}
			}
			else if (nextChar == '\t')
			{
				// Autocomplete:
				int i;
				for(i = 0; i < SHELL_MAX_PROGRAMS; i++)
				{
					int namelen = fast_strlen(shell_progMap[i].name);
					// If the program is valid
					if(namelen)
					{
						// If the current line contents are less or equal to the name length
						if(shell_lineBufferIndex <= namelen)
						{
							if(fast_memcmp(shell_lineBuffer, shell_progMap[i].name, shell_lineBufferIndex) == 0)
							{
								// They didn't mean this one
								if(shell_lineBufferIndex == namelen)
								{
									continue;
								}

								fast_memcpy(shell_lineBuffer, shell_progMap[i].name, namelen);

								while(shell_lineBufferIndex--)
									s_putc(SHELL_BACKSPACE);
								sys_write(STDOUT, (uint8_t*)shell_lineBuffer, namelen);
								shell_lineBufferIndex = shell_lineBufferInsertIndex = namelen;
							}
						}
					}
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
					sys_write(STDOUT, _bsp_seq, 3);
				}
				else
				{
					s_putc(SHELL_BELL_CHAR);
				}
				continue;
			}
			// Handle special character
			else if (nextChar < ' ')
			{
				if (nextChar == SHELL_ESC_CHAR)
				{
					if (shell_readChar() == '[')
					{
						char control_char;
						switch (control_char = shell_readChar())
						{
//            			case 'A':
//            				continue;
//            				break;
//            			case 'B':
//            				continue;
//            				break;
						case '4':
							shell_readChar(); // Consume '~'
							shell_lineBufferInsertIndex = shell_lineBufferIndex;
							s_puts("\x1B[4~");
							continue;
							break;
						case '1':
							shell_readChar(); // Consume '~'
							shell_lineBufferInsertIndex = 0;
							s_puts("\x1B[1~");
							continue;
							break;
						case 'D':
							if (shell_lineBufferInsertIndex > 0)
							{
								shell_lineBufferInsertIndex--;
								s_puts(SHELL_LARROW_SEQ);
							}
							else
							{
								s_putc(SHELL_BELL_CHAR);
							}
							continue;
							break;
						case 'C':
							if (shell_lineBufferInsertIndex
									< shell_lineBufferIndex)
							{
								shell_lineBufferInsertIndex++;
								s_puts(SHELL_RARROW_SEQ);
							}
							else
							{
								s_putc(SHELL_BELL_CHAR);
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
				s_putc(SHELL_BELL_CHAR);
				shell_lineBufferIndex--;
				shell_lineBufferInsertIndex--;
				continue;
			}

			s_putc(nextChar);
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

	fast_free(shimparams);

	//sys_unlock(&shim_print_lock);

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
	s_puts(printbuf);
	s_puts(job->program);
	s_puts("\'\r\n");

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
			fast_snprintf(printbuf, 32, "[%d] %d running\t'", i, (int)shell_jobs[i].thread);
			s_puts(printbuf);
			s_puts(shell_jobs[i].program);
			s_puts("\'\r\n");
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

tid_t process_command(ptnode_t* node, fd_t stdin, fd_t stdout, fd_t stderr)
{
	int progidx = shell_getProgIdx(node->tlist[0]);
	if (progidx >= 0)
	{
		shell_run_shim_params_t* shim_params = (shell_run_shim_params_t*)fast_alloc(sizeof(shell_run_shim_params_t));
		shim_params->func = shell_progMap[progidx].prog_main;
		shim_params->argc = node->tlsiz;
		shim_params->argv = node->tlist;

		//shell_substitute_vars();



		tid_t tid = sys_spawn2(shell_run_shim, shim_params, stdin, stdout, stderr);

		//TODO: Fix max jobs problem; handle job == NULL properly
		return tid;

	}
	else
	{
		s_puts(shell_error_unknown_command);
		s_puts(shell_argBuffer[0]);
		s_puts("\'\r\n");
		return 0;
	}
}

fd_t tempfds[16];
uint8_t tempfdidx = 0;

tid_t process_tree(ptnode_t* node, fd_t stdin, fd_t stdout, fd_t stderr)
{
	fd_t tempfd = FD_INVALID;
	tid_t lefttid = 0, righttid = 0;
	switch(node->type)
	{
	case NONE:
		return 0;
		break;
	case PIPE:
		tempfd = sys_popen(32);
		if (tempfd == FD_INVALID)
			return 0;
		lefttid = process_tree(node->left, stdin, tempfd, stderr);
		righttid = process_tree(node->right, tempfd, stdout, stderr);
		break;
	case REDIR_L:
		// Input file
		tempfd = sys_open(node->right->tlist[0], FMODE_R, 0);
		if (tempfd == FD_INVALID)
			return 0;
		lefttid = process_tree(node->left, tempfd, stdout, stderr);
		break;
	case REDIR_R:
		// Output file
		tempfd = sys_open(node->right->tlist[0], FMODE_W, FFLAG_CREAT);
		if (tempfd == FD_INVALID)
			return 0;
		lefttid = process_tree(node->left, stdin, tempfd, stderr);
		break;
	case COMMAND:
		return process_command(node, stdin, stdout, stderr);
		break;
	default:
		return 0;
	}

	if(tempfd != FD_INVALID)
		tempfds[tempfdidx++] = tempfd;

	if (righttid)
		return righttid;
	if (lefttid)
		return lefttid;
	return 0;
}

void print_toklist(char** tokenlist, int32_t numtoks)
{
    int32_t i = 0;
    while(i < numtoks)
    {
        s_puts(tokenlist[i++]);
        s_putc('+');
    }
}

void ptnode_tree_walk(ptnode_t* root)
{
	if (!root)
		return;

	switch (root->type)
	{
	case NONE:
		return;
	case COMMAND:
		s_puts("Command <");
		print_toklist(root->tlist, root->tlsiz);
		s_puts(">");
		return;
	case FILENAME:
		s_puts("Filename <");
		print_toklist(root->tlist, root->tlsiz);
		s_puts(">");
		return;
	case PIPE:
		s_puts("Pipe <");
		ptnode_tree_walk(root->left);
		s_puts(",");
		ptnode_tree_walk(root->right);
		s_puts(">");
		return;
	case COND_AND:
		s_puts("And <");
		ptnode_tree_walk(root->left);
		s_puts(",");
		ptnode_tree_walk(root->right);
		s_puts(">");
		return;
	case COND_OR:
		s_puts("Or <");
		ptnode_tree_walk(root->left);
		s_puts(",");
		ptnode_tree_walk(root->right);
		s_puts(">");
		return;
	case REDIR_L:
		s_puts("Input <");
		ptnode_tree_walk(root->left);
		s_puts(",");
		ptnode_tree_walk(root->right);
		s_puts(">");
		return;
	case REDIR_R:
		s_puts("Output <");
		ptnode_tree_walk(root->left);
		s_puts(",");
		ptnode_tree_walk(root->right);
		s_puts(">");
		return;
	default:
		return;
	}
}

void shell_processLine(void)
{
	bool background = false;

	char* tokenlist[TOKENLIST_LENGTH];
	int32_t numtoks = tokenize_line(shell_lineBuffer, tokenlist);
	ptnode_t blist[BLIST_SIZE];
	ptnode_t* root = parse_line_toks(tokenlist, numtoks, blist, BLIST_SIZE,
			&background);

	//ptnode_tree_walk(root);

	tid_t waittid = process_tree(root, 0, 1, 2);

	// TODO: Handle waiting; pass back tid somehow
	if (waittid)
	{
		if (!background)
		{
			lastret = sys_wait(waittid);
		}
		else
		{
			background = false;
			shell_job_t* job = shell_allocJob();
			if (job != NULL)
			{
				job->thread = waittid;
				job->program = shell_progMap[0].name;
				sys_spawn(shell_job_waiter, job);
				char printbuf[32];
				fast_snprintf(printbuf, 32, "[%d] %d\r\n",
						(int) (job - shell_jobs), (int) waittid);
				s_puts(printbuf);
			}
			else
			{
				s_puts("Exceeded max jobs!\r\n");
			}
		}
	}

	uint8_t i;
	for(i = 0; i < tempfdidx; i++)
		sys_close(tempfds[i]);
	tempfdidx = 0;
}

