#include <stdlib.h>
#if __STDC_HOSTED__ == 0
void kpanic(const char* message);
#endif

__attribute__((__noreturn__))
void abort(void)
{
#if __STDC_HOSTED__ == 1
    //TODO: Call exit with suitable errno
#else
    //Panic
    kpanic("Abnormal exit");
    while(1) asm("hlt");
#endif    
    __builtin_unreachable();
}
