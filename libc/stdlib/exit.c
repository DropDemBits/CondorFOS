#include <stdlib.h>
#include <stdio.h>
#ifdef __LIBK_BUILD
//extern void kexit(int status);
#include <condor.h>
#endif

//__attribute__((__noreturn__))
void exit(int status)
{
    #ifdef __LIBK_BUILD
        //Shutdown? (for now go into kexit)
        kexit(status);
    #else
        //Normal app exit
        //TODO: Properly exit program
        printf("exit(%d)\n", status);
    #endif

    //while(1) asm("hlt");
    //__builtin_unreachable();
}
