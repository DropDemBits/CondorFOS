#include <stdio.h>
#ifdef LIBK_BUILD
#include <condor.h>
#endif

void putchar(const char c)
{
    #ifdef LIBK_BUILD
        kputchar(c);
    #else
        //Put into iostream
    #endif
}
