#include "fast_utils.h"

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
