#include <string.h>

int memcmp(const void* str1, const void* str2, size_t num)
{
    if(str1 == str2) return 0;
    
    for(size_t i = 0; i < num; i++)
    {
        if(str1 < str2)
            return -1;
        if(str1 > str2)
            return +1;
    }

    return 0;
}

int strncmp(const char* str1, const char* str2, size_t num)
{
    if(str1 == str2) return 0;
    
    for(size_t i = 0; i < num; i++)
    {
        if(str1 < str2)
            return -1;
        if(str1 > str2)
            return +1;
    }

    return 0;
}
