#include <stdlib.h>
#ifdef LIBK_BUILD
#include <condor.h>
#endif

void abort(void)
{
    #ifdef LIBK_BUILD
        //Panic
        kpanic("Abnormal exit");
    #else
        //Abnormal app exit
    #endif
}
