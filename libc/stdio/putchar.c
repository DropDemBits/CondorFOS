#include <stdio.h>
#ifdef __LIBK_BUILD
//extern void kputchar(const char c);
#include <condor.h>
#endif

int putchar(int ic)
{
    #ifdef __LIBK_BUILD
        kputchar(ic);
    #else
        //Put into iostream
    #endif
    return ic;
}
