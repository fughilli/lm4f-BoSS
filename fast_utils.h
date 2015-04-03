#ifndef _FAST_UTILS_H_
#define _FAST_UTILS_H_

#include <stdint.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

#define ATOMIC

#ifndef size_t
typedef uint32_t size_t;
#endif

#define __ALLOC_MEM_SIZE (4096)

void fast_memcpy(void* dst, const void* src, size_t sz);
void fast_memset(void* dst, uint8_t val, size_t sz);
size_t fast_strlen(const char* str);

void fast_snprintf(char* buf, size_t bufsiz, const char* fmt, ...);

void* fast_alloc(size_t sz);
void fast_free(void* buf);

#endif // _FAST_UTILS_H_

