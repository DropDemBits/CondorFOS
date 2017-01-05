#include <stdlib.h>
#ifdef __LIBK_BUILD
//extern void kpanic(const char* message);
#include <condor.h>
#endif

__attribute__((__noreturn__))
void abort(void)
{
    #ifdef __LIBK_BUILD
        //Panic
        kpanic("Abnormal exit");
    #else
        //Abnormal app exit
    #endif
    while(1) asm("hlt");
    __builtin_unreachable();
}
