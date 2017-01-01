#include <stdlib.h>
#ifdef LIBK_BUILD
#include <condor.h>
#endif

void exit(int status)
{
    #ifdef LIBK_BUILD
        //Shutdown? (for now go into kexit)
        kexit(status);
    #else
        //Normal app exit
    #endif
}
