#include <string.h>

void* memset(void* str, const int c, size_t num)
{
    unsigned char* strdest = (unsigned char*) str;
    for(int i = 0; i < num; i++) strdest[i] = c;
    return strdest;
}
