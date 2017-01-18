#include <string.h>
#include <math.h>

void* memcpy(void* dest, const void* src, size_t num)
{
    //TODO: implement faster copy, and align
    if(dest == src) return dest;
    
    unsigned char *cdest = (unsigned char*) dest;
    unsigned char *csrc = (unsigned char*) src;
    for(size_t i = 0; i < num; i++)
        cdest[i] = csrc[i];
    return dest;
}

void* strcopy(char* dest, const char* src)
{
    if(dest == src) return dest;
    
    size_t num = max(strlen(src), strlen(dest));
    if(dest < src)
        for(size_t i = 0; i < num; i++)
            dest[i] = src[i];
    else
        for(size_t i = num; i != 0; i--)
            dest[i-1] = src[i-1];
    return dest;
}

void* strncopy(char* dest, const char* src, size_t num)
{
    if(dest == src) return dest;
    
    if(dest < src)
        for(size_t i = 0; i < num || !src[i]; i++)
            dest[i] = src[i];
    else
        for(size_t i = num; i != 0 || !src[i]; i--)
            dest[i-1] = src[i-1];
    return dest;
}
