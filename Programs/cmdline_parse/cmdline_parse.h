/*
 * cmdline_parse.h
 *
 *  Created on: May 7, 2015
 *      Author: Kevin
 */

#ifndef CMDLINE_PARSE_H_
#define CMDLINE_PARSE_H_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define TOKENLIST_LENGTH (16)
#define BLIST_SIZE (16)

typedef struct ptnode
{
    enum
    {
        NONE,
        ALLOCD,
        COMMAND,
        PIPE,
        COND_AND,
        COND_OR,
        REDIR_L,
        REDIR_R,
        FILENAME
    } type;
    char** tlist;
    int32_t tlsiz;
    struct ptnode* left;
    struct ptnode* right;
} ptnode_t;

typedef struct
{
    ptnode_t* list;
    int32_t banklen;
    int32_t nextfree;
} ptnode_bank_t;

ptnode_t* parse_line_toks( char* * tokenlist, int32_t numtoks, ptnode_t* blist, int32_t blistsiz, bool* bgflag);
int32_t tokenize_line(char* linebuffer,  char** tokenlist);

#endif /* CMDLINE_PARSE_H_ */
