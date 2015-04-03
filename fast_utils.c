#include "fast_utils.h"
#include <stdarg.h>
#include <stdbool.h>

uint8_t alloc_mem[__ALLOC_MEM_SIZE];

// TODO: build an actual allocator.

uint32_t alloc_off = 0;

void* fast_alloc(size_t sz)
{
	uint8_t* mem = &alloc_mem[alloc_off];
	alloc_off += sz;
	return (void*)mem;
}

void fast_free(void* buf)
{
	; // Do nothing.
}

void fast_memcpy(void* dst, const void* src, size_t sz)
{
    do
    {
        --sz;
        ((uint8_t*)dst)[sz] = ((uint8_t*)src)[sz];
    } while(sz);
}

void fast_memset(void* dst, uint8_t val, size_t sz)
{
    do
    {
        --sz;
        ((uint8_t*)dst)[sz] = val;
    } while(sz);
}

size_t fast_strlen(const char* str)
{
	size_t ret = 0;
	while(str[ret])
		ret++;
	return ret;
}

static int snfmtui(char* buf, size_t bufsiz, unsigned long i)
{
    if(bufsiz == 0)
        return 0;

    if(i == 0)
    {
        if(bufsiz >= 2)
        {
            buf[0] = '0';
            buf[1] = 0;
            return 1;
        }
        else
        {
            buf[0] = 0;
            return 0;
        }
    }

    size_t bufpos = 0, swappos = 0, ret = 0;

    while(bufpos < bufsiz && i != 0)
    {
        buf[bufpos++] = '0' + (i % 10);
        i /= 10;
    }

    if(bufpos == bufsiz)
    {
        bufpos--;
    }

    buf[bufpos] = '\0';
    ret = bufpos;
    bufpos--;

    while(swappos < bufpos)
    {
        char temp = buf[swappos];
        buf[swappos++] = buf[bufpos];
        buf[bufpos--] = temp;
    }

    return ret;
}

static int snfmti(char* buf, size_t bufsiz, long i)
{
    if(i < 0)
    {
        buf[0] = '-';
        return snfmtui(buf+1, bufsiz-1, -i)+1;
    }
    return snfmtui(buf, bufsiz, i);
}

__attribute__((format(printf,3,4)))
void fast_snprintf(char* buf, size_t bufsiz, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	size_t bufpos = 0;
	size_t fmtpos = 0;

	size_t fmtlen = fast_strlen(fmt);

	bool sym_unsigned = false;

	while(bufpos < bufsiz && fmtpos < fmtlen)
	{
		// Look for escape character
		if(fmt[fmtpos] == '%')
		{
			fmtpos++;
			if (fmtpos == fmtlen)
			{
				buf[bufpos] = '\0';
				return;
			}

			_match_fchar:
			switch(fmt[fmtpos])
			{
			case '%':
				// Save '%' to string
				buf[bufpos++] = fmt[fmtpos++];
				continue;
			case 'u':
				// This symbol is unsigned
				sym_unsigned = true;
				goto _match_fchar;
			case 'd':
				// Print an int from the args
				if(sym_unsigned)
				{
					unsigned long argval;
					argval = va_arg(ap, unsigned long);
					bufpos += snfmtui(&buf[bufpos], bufsiz-bufpos, argval);
					sym_unsigned = false;
				}
				else
				{
					long argval;
					argval = va_arg(ap, long);
					bufpos += snfmti(&buf[bufpos], bufsiz-bufpos, argval);
				}
				fmtpos++;
				continue;
			}
		}
		buf[bufpos++] = fmt[fmtpos++];
	}

	if(bufpos == bufsiz)
		bufpos--;
	buf[bufpos] = '\0';

	va_end(ap);
}
