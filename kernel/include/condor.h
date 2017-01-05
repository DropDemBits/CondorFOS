#include <stddef.h>

#ifndef CONDOR_KERNEL_H
#define CONDOR_KERNEL_H

//Macros

//Kernel version
#define KENREL_TYPE_ALPHA 0
#define KENREL_TYPE_BETA 1
#define KENREL_TYPE_RC 2
#define KENREL_TYPE_RELEASE 3

#define KENREL_MAJOR 0
#define KENREL_MINOR 1
#define KENREL_PATCH 0
#define KERNEL_TYPE  KENREL_TYPE_ALPHA

/* Bindings for libk */
void kexit(int status);
void kpanic(const char* message);
void kputchar(const char c);

inline int kstrlen(const char* str)
{
    size_t length = 0;
    while(str[length]) length++;
    return length;
}

#endif
