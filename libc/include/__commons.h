#include <stddef.h>

//Commons
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __GNU_C__
/* GNU Compiler specifics */
#else
#define __attribute__(a)
#endif

#ifdef NULL
#   undef NULL
#   define NULL (void*)0
#endif

#ifdef __cplusplus
}
#endif
