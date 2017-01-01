#include <stdio.h>
#ifdef LIBK_BUILD
#include <condor.h>
#endif

void puts(const char *str)
{
    while(*str)
    {
        putchar(*str);
        str++;
    }
}
