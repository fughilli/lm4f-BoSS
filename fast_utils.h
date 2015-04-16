#ifndef _FAST_UTILS_H_
#define _FAST_UTILS_H_

#include <stdint.h>
#include <stdbool.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

#define ATOMIC

#ifndef size_t
typedef uint32_t size_t;
#endif

#define __ALLOC_MEM_SIZE (4096)

void fast_memcpy(void* dst, const void* src, size_t sz);
void fast_memmove(void* dst, const void* src, size_t sz);
void fast_memset(void* dst, uint8_t val, size_t sz);
int fast_memcmp(const void* ptr1, const void* ptr2, size_t sz);

size_t fast_strlen(const char* str);
size_t fast_strcpy(char* stra, const char* strb);
int fast_strcmp(char* stra, const char* strb);



void fast_snprintf(char* buf, size_t bufsiz, const char* fmt, ...);

long fast_sntol(const char* str, size_t sz, unsigned int base, bool* succ);
unsigned long fast_sntoul(const char* str, size_t sz, unsigned int base, bool* succ);
float fast_sntof(const char* str, size_t sz, unsigned int base, bool* succ);
double fast_sntod(const char* str, size_t sz, unsigned int base, bool* succ);

int fast_snfmtui(char* buf, size_t bufsiz, unsigned long i, unsigned long base);
int fast_snfmti(char* buf, size_t bufsiz, long i, unsigned long base);

unsigned long fast_nextmulof(unsigned long val, unsigned long q);

void* fast_alloc(size_t sz);
void fast_free(void* buf);

#endif // _FAST_UTILS_H_


