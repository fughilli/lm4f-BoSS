/*
 * cmdline_parse.c
 *
 *  Created on: May 7, 2015
 *      Author: Kevin
 */

#include "cmdline_parse.h"
#include "../osprogram.h"

void ptnode_bank_init(ptnode_bank_t* bank, ptnode_t* list, int32_t banklen)
{
    bank->nextfree = 0;
    bank->banklen = banklen;
    bank->list = list;

    int i;
    for(i = 0; i < banklen; i++)
    {
        list[i].type = NONE;
        list[i].left = NULL;
        list[i].right = NULL;
        list[i].tlist = NULL;
        list[i].tlsiz = 0;
    }
}

ptnode_t* ptnode_bank_alloc(ptnode_bank_t* bank)
{
    if(!bank)
        return NULL;

    int i;
    for(i = 0; i < (bank->banklen); i++)
    {
        if((bank->list[i].type) == NONE)
        {
            bank->list[i].type = ALLOCD;
            return &(bank->list[i]);
        }
    }
    return NULL;
}

void ptnode_bank_free(ptnode_t* node)
{
    if(node)
    {
        node->type = NONE;
    }
}

bool parse_command_string(char** tokenlist, int32_t numtoks, ptnode_t* parent, ptnode_bank_t* bank);

void shell_error(const char* strng)
{
    s_puts(strng);
}

bool split_on_first(char** tokenlist,
                    int32_t numtoks,
                    char* tok,
                    char*** ltoklist,
                    int32_t* ltoklsiz,
                    char*** rtoklist,
                    int32_t* rtoklsiz)
{
    int32_t i = 0;
    while(i < numtoks)
    {
        if(fast_strcmp(tokenlist[i], tok) == 0)
        {
            (*ltoklist) = tokenlist;
            (*ltoklsiz) = i;
            (*rtoklist) = &tokenlist[i + 1];
            (*rtoklsiz) = (numtoks - i - 1);

            return true;
        }

        i++;
    }
    return false;
}

bool parse_command( char* * tokenlist, int32_t numtoks, ptnode_t* parent, ptnode_bank_t* bank)
{
    ptnode_t* commnode = parent;

    if(!commnode)
    {
        shell_error("Ran out of ptnodes!");
        return false;
    }

    commnode->type = COMMAND;
    commnode->tlist = tokenlist;
    commnode->tlsiz = numtoks;

    return true;
}

bool parse_filename( char* * tokenlist, int32_t numtoks, ptnode_t* parent, ptnode_bank_t* bank)
{
    if(numtoks == 1)
    {
        ptnode_t* commnode = parent;

        if(!commnode)
        {
            shell_error("Ran out of ptnodes!");
            return false;
        }

        commnode->type = FILENAME;
        commnode->tlist = tokenlist;
        commnode->tlsiz = numtoks;

        return true;
    }

    return false;
}


bool parse_conditional( char* * tokenlist, int32_t numtoks, ptnode_t* parent, ptnode_bank_t* bank)
{

    ptnode_t *lside = ptnode_bank_alloc(bank), *rside = ptnode_bank_alloc(bank);

    if(!lside || !rside)
    {
        shell_error("Ran out of ptnodes!");
        return false;
    }

    #define AND_SUCC 1
    #define OR_SUCC 2

    int succ = 0;

    if(split_on_first(tokenlist, numtoks, "&&", &lside->tlist, &lside->tlsiz, &rside->tlist, &rside->tlsiz))
        succ = AND_SUCC;
    else if(split_on_first(tokenlist, numtoks, "||", &lside->tlist, &lside->tlsiz, &rside->tlist, &rside->tlsiz))
        succ = OR_SUCC;

    if(succ)
    {
        parent->left = lside;
        parent->right = rside;

        if(succ == AND_SUCC)
            parent->type = COND_AND;
        else if(succ == OR_SUCC)
            parent->type = COND_OR;

        bool ret = parse_command_string(lside->tlist, lside->tlsiz, lside, bank);
        ret = ret && parse_command_string(rside->tlist, rside->tlsiz, rside, bank);
        return ret;
    }

    ptnode_bank_free(lside);
    ptnode_bank_free(rside);
    return false;
}

bool parse_redirect( char* * tokenlist, int32_t numtoks, ptnode_t* parent, ptnode_bank_t* bank)
{
    ptnode_t *lside = ptnode_bank_alloc(bank), *rside = ptnode_bank_alloc(bank);

    #define INPUT_REDIR 1
    #define OUTPUT_REDIR 2

    int succ = 0;

    if(split_on_first(tokenlist, numtoks, "<", &lside->tlist, &lside->tlsiz, &rside->tlist, &rside->tlsiz))
        succ = INPUT_REDIR;
    else if(split_on_first(tokenlist, numtoks, ">", &lside->tlist, &lside->tlsiz, &rside->tlist, &rside->tlsiz))
        succ = OUTPUT_REDIR;

    if(succ)
    {
        parent->left = lside;
        parent->right = rside;

        if(succ == INPUT_REDIR)
            parent->type = REDIR_L;
        else if(succ == OUTPUT_REDIR)
            parent->type = REDIR_R;

        bool ret = parse_command_string(lside->tlist, lside->tlsiz, lside, bank);
        ret = ret && parse_filename(rside->tlist, rside->tlsiz, rside, bank);
        return ret;
    }

    ptnode_bank_free(lside);
    ptnode_bank_free(rside);
    return false;
}

bool parse_pipe( char* * tokenlist, int32_t numtoks, ptnode_t* parent, ptnode_bank_t* bank)
{
    ptnode_t *lside = ptnode_bank_alloc(bank), *rside = ptnode_bank_alloc(bank);

    if(split_on_first(tokenlist, numtoks, "|", &lside->tlist, &lside->tlsiz, &rside->tlist, &rside->tlsiz))
    {
        parent->type = PIPE;
        parent->left = lside;
        parent->right = rside;

        bool ret = parse_command_string(lside->tlist, lside->tlsiz, lside, bank);
        ret = ret && parse_command_string(rside->tlist, rside->tlsiz, rside, bank);
        return ret;
    }

    ptnode_bank_free(lside);
    ptnode_bank_free(rside);
    return false;
}

bool parse_command_string(char** tokenlist, int32_t numtoks, ptnode_t* parent, ptnode_bank_t* bank)
{
    /*
     * Precedence:
     * Conditionals
     * Pipes
     * Redirects
     *      -Command, Word
     * Commands
     */

    if(!parse_conditional(tokenlist, numtoks, parent, bank))
    {
        if(!parse_pipe(tokenlist, numtoks, parent, bank))
        {
            if(!parse_redirect(tokenlist, numtoks, parent, bank))
            {
                // try next
                if(!parse_command(tokenlist, numtoks, parent, bank))
                {
                    return false;
                }
            }
        }
    }

    return true;
}


// Returns root
ptnode_t* parse_line_toks( char* * tokenlist, int32_t numtoks, ptnode_t* blist, int32_t blistsiz, bool* bgflag)
{
    char **ltoklist, **rtoklist;
    int32_t ltoklsiz, rtoklsiz;

    ptnode_bank_t bank;
    ptnode_bank_init(&bank, blist, blistsiz);

    ptnode_t* root = ptnode_bank_alloc(&bank);

    if(bgflag)
    	(*bgflag) = false;

    bool succ = split_on_first(tokenlist, numtoks, "&", &ltoklist, &ltoklsiz, &rtoklist, &rtoklsiz);

    if(succ && rtoklsiz)
    {
        shell_error("Unexpected \'&\'");
        return NULL;
    }

    if(succ)
    {
        succ = parse_command_string(ltoklist, ltoklsiz, root, &bank);
        if(bgflag)
        	(*bgflag) = true;
    }
    else
    {
        succ = parse_command_string(tokenlist, numtoks, root, &bank);
    }

    if(succ)
    {
        return root;
    }
    else
    {
        return NULL;
    }
}

int32_t tokenize_line(char* linebuffer,  char** tokenlist)
{
	char* head = linebuffer;
	int32_t numtoks = 0;
	char quotechar = 0;
	bool quotestrflag = false;

	while ((*linebuffer) != '\0')
	{
		if (quotechar)
		{
			if ((*linebuffer) == quotechar)
			{
				quotechar = 0;
				quotestrflag = true;
			}
			else
			{
				linebuffer++;
				continue;
			}
		}

		bool isquote = ((*linebuffer) == '\'' || (*linebuffer) == '\"');
		if (isquote && !quotestrflag)
		{
			quotechar = (*linebuffer);
			head++;
		}
		else if (((*linebuffer) == ' ') || (isquote && quotestrflag))
		{
			quotestrflag = false;
			if ((numtoks + 1) > TOKENLIST_LENGTH)
			{
				return -1;
			}

			(*linebuffer) = '\0';
			(*tokenlist) = head;

			if ((*head) != '\0')
			{
				tokenlist++;
				numtoks++;
			}
			head = linebuffer + 1;
		}

		linebuffer++;
	}

	if (head < linebuffer)
	{
		(*tokenlist) = head;
		numtoks++;
	}

	return numtoks;
}

//ptnode_t* parse_line(char* linebuffer, bool* bgflag)
//{
//	char* tokenlist[TOKENLIST_LENGTH];
//	int32_t numtoks = tokenize_line(linebuffer, tokenlist);
//	ptnode_t blist[BLIST_SIZE];
//	return parse_line_toks(tokenlist, numtoks, blist, BLIST_SIZE, bgflag);
//}

