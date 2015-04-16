#include "fast_utils.h"
#include <stdarg.h>
#include <stdbool.h>

uint8_t alloc_mem[__ALLOC_MEM_SIZE];

// TODO: build an actual allocator.

uint32_t alloc_off = 0;

#define MAX_BASE (16)
const char __arb_base_digits[] = "0123456789ABCDEF"; //"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+/";

#define __digits_val_rev_lookup(_digit_) ((((_digit_) >= '0')&&((_digit_) <= '9'))?((_digit_)-'0'):((((_digit_) >= 'A')&&((_digit_) <= 'F'))?((_digit_)-('A'-10)):(-1)))

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
    if(dst == src || sz == 0)
        return;

    do
    {
        --sz;
        ((uint8_t*)dst)[sz] = ((uint8_t*)src)[sz];
    } while(sz);
}

void fast_memmove(void* dst, const void* src, size_t sz)
{
    size_t iter = 0;

    if(dst == src || sz == 0)
        return;

    if(dst > src)
        do
        {
            --sz;
            ((uint8_t*)dst)[sz] = ((uint8_t*)src)[sz];
        }
        while(sz);
    else
        do
        {
            ((uint8_t*)dst)[iter] = ((uint8_t*)src)[iter];
        }
        while(iter++ < sz);
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

int fast_memcmp(const void* ptr1, const void* ptr2, size_t sz)
{
	int8_t *a, *b;
	a = (int8_t*)ptr1;
	b = (int8_t*)ptr2;
	int ret = 0;

	while(sz && (ret == 0))
	{
		sz--;
		ret += (*a);
		ret -= (*b);
	}

	return ret;
}

size_t fast_strcpy(char* stra, const char* strb)
{
	size_t ret = fast_strlen(strb);
	fast_memcpy(stra, strb, ret);
	return ret;
}

int fast_strcmp(char* stra, const char* strb)
{
	int ret = 0;
	while ((ret == 0) && (*stra) && (*strb))
	{
		ret += (*stra);
		ret -= (*strb);
		stra++;
		strb++;
	}
	return ret;
}

long fast_sntol(const char* str, size_t sz, unsigned int base, bool* succ)
{
	long ret = 0;

	size_t i = 0;
	bool sign = false;

	if (succ)
	{
		(*succ) = false;
	}

	if (base > MAX_BASE || sz == 0)
		return 0;

	if (sz >= 2 && str[0] == '-')
	{
		sign = true;
		i = 1;
	}

	for (; i < sz; i++)
	{
		int16_t charval = __digits_val_rev_lookup(str[i]);
		if (charval < 0 || charval >= base)
		{
			return 0;
		}

		ret *= base;
		ret += charval;
	}

	if (succ)
	{
		(*succ) = true;
	}

	if (sign)
		ret = -ret;
	return ret;
}

unsigned long fast_sntoul(const char* str, size_t sz, unsigned int base, bool* succ)
{
	unsigned long ret = 0;

	size_t i;

	if (succ)
	{
		(*succ) = false;
	}

	if (base > MAX_BASE || sz == 0)
		return 0;

	for (i = 0; i < sz; i++)
	{
		int16_t charval = __digits_val_rev_lookup(str[i]);
		if (charval < 0 || charval >= base)
		{
			return 0;
		}

		ret *= base;
		ret += charval;
	}

	if (succ)
	{
		(*succ) = true;
	}

	return ret;
}

float fast_sntof(const char* str, size_t sz, unsigned int base, bool* succ)
{
	float ret = 0;
	float retfpart = 0;

	size_t i = 0;
	bool sign = false;

	if (succ)
	{
		(*succ) = false;
	}

	if (base > MAX_BASE || sz == 0)
		return 0;

	if (sz >= 2 && str[0] == '-')
	{
		sign = true;
		i = 1;
	}

	for (; i < sz; i++)
	{
		if (str[i] == '.')
			break;
		int16_t charval = __digits_val_rev_lookup(str[i]);
		if (charval < 0 || charval >= base)
		{
			return 0;
		}

		ret *= base;
		ret += charval;
	}

	if (str[i])
	{

		int j = fast_strlen(&str[i + 1]);

		if ((j + i + 1) > sz)
			return 0;

		for (; j > 0; j--)
		{
			int16_t charval = __digits_val_rev_lookup(str[i + j]);
			if (charval < 0 || charval >= base)
			{
				return 0;
			}

			retfpart += charval;
			retfpart /= base;
		}

	}

	if (succ)
	{
		(*succ) = true;
	}

	ret += retfpart;

	if (sign)
		ret = -ret;
	return ret;
}

double fast_sntod(const char* str, size_t sz, unsigned int base, bool* succ)
{
	double ret = 0;
	double retfpart = 0;

	size_t i = 0;
	bool sign = false;

	if (succ)
	{
		(*succ) = false;
	}

	if (base > MAX_BASE || sz == 0)
		return 0;

	if (sz >= 2 && str[0] == '-')
	{
		sign = true;
		i = 1;
	}

	for (; i < sz; i++)
	{
		if (str[i] == '.')
			break;
		int16_t charval = __digits_val_rev_lookup(str[i]);
		if (charval < 0 || charval >= base)
		{
			return 0;
		}

		ret *= base;
		ret += charval;
	}

	if (str[i])
	{

		int j = fast_strlen(&str[i + 1]);

		if ((j + i + 1) > sz)
			return 0;

		for (; j > 0; j--)
		{
			int16_t charval = __digits_val_rev_lookup(str[i + j]);
			if (charval < 0 || charval >= base)
			{
				return 0;
			}

			retfpart += charval;
			retfpart /= base;
		}

	}

	if (succ)
	{
		(*succ) = true;
	}

	ret += retfpart;

	if (sign)
		ret = -ret;
	return ret;
}

unsigned long fast_nextmulof(unsigned long val, unsigned long q)
{
	// base cases
	if(q == 0)
		return 0;
	if(q == 1)
		return val;

	unsigned long modulo = val % q;

	if(modulo == 0)
		return val;

	return (val - modulo) + q;
}

int fast_snfmtui(char* buf, size_t bufsiz, unsigned long i, unsigned long base)
{
    if(bufsiz == 0 || base > MAX_BASE)
        return 0;

    if(i == 0)
    {
        if(bufsiz >= 2)
        {
            buf[0] = __arb_base_digits[0];
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
        buf[bufpos++] = __arb_base_digits[(i % base)];
        i /= base;
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

int fast_snfmti(char* buf, size_t bufsiz, long i, unsigned long base)
{
    if(i < 0)
    {
        buf[0] = '-';
        return fast_snfmtui(buf+1, bufsiz-1, -i, base)+1;
    }
    return fast_snfmtui(buf, bufsiz, i, base);
}

__attribute__((format(printf,3,4)))
void fast_snprintf(char* buf, size_t bufsiz, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	size_t bufpos = 0;
	size_t fmtpos = 0;

	size_t fmtlen = fast_strlen(fmt);

	size_t precision = 0;

	bool padwzeros = false;

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

			size_t stofprec = fmtpos;

			while('0' <= fmt[fmtpos] && fmt[fmtpos] <= '9')
            {
                if(fmt[fmtpos] == '0' && !padwzeros)
                {
                    padwzeros = true;
                    fmtpos++;
                    stofprec++;
                    continue;
                }

                fmtpos++;
            }

            bool succ;

            precision = fast_sntoul(&fmt[stofprec], fmtpos - stofprec, 10, &succ);
            if(!succ)
            {
                precision = 0;
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
				fmtpos++;
				goto _match_fchar;
			case 'd':
			case 'x':
			{
				unsigned long base;
				switch(fmt[fmtpos])
				{
				case 'd':
					base = 10;
					break;
				case 'x':
				    sym_unsigned = true;
					base = 16;
					break;
				}
				// Print an int from the args, formatted as decimal or hex
				if(sym_unsigned)
				{
					unsigned long argval;
					argval = va_arg(ap, unsigned long);
					if(!precision)
                        bufpos += fast_snfmtui(&buf[bufpos], bufsiz-bufpos, argval, base);
                    else
                    {
                        size_t intstrsiz = fast_snfmtui(&buf[bufpos], bufsiz-bufpos, argval, base);
                        if(intstrsiz < precision)
                        {
                            if((bufsiz-bufpos) >= precision)
                            {
                                fast_memmove(&buf[bufpos + (precision - intstrsiz)], &buf[bufpos], intstrsiz);
                                fast_memset(&buf[bufpos], (uint8_t)((padwzeros)?('0'):(' ')), precision-intstrsiz);
                                bufpos += precision;
                            }
                            else
                                break;
                        }
                        else
                            bufpos += intstrsiz;
                    }
					sym_unsigned = false;
				}
				else
				{
					long argval;
					argval = va_arg(ap, long);
					if(!precision)
                        bufpos += fast_snfmti(&buf[bufpos], bufsiz-bufpos, argval, base);
                    else
                    {
                        size_t intstrsiz = fast_snfmti(&buf[bufpos], bufsiz-bufpos, argval, base);
                        if(intstrsiz < precision)
                        {
                            if((bufsiz-bufpos) >= precision)
                            {
                                fast_memmove(&buf[bufpos + (precision - intstrsiz)], &buf[bufpos], intstrsiz);
                                fast_memset(&buf[bufpos], (uint8_t)((padwzeros)?('0'):(' ')), precision-intstrsiz);
                                bufpos += precision;
                            }
                            else
                                break;
                        }
                        else
                            bufpos += intstrsiz;
                    }
				}
				fmtpos++;
				continue;
			}
			}
		}
		buf[bufpos++] = fmt[fmtpos++];
	}

	if(bufpos == bufsiz)
		bufpos--;
	buf[bufpos] = '\0';

	va_end(ap);
}

