#include <stddef.h>
#include <stdlib.h>
#include <kernel/liballoc.h>

#ifndef _CONDOR_H
#define _CONDOR_H

//Macros

#define KERNEL_ALPHA_STR  "-alpha"
#define KERNEL_BETA_STR   "-beta"
#define KERNEL_RC_STR     "-rc"
#define KERNEL_RELEASE_STR "\0"

#define KERNEL_VERSION_FORMATER "%ld.%ld.%ld%s"

//Kernel version
#define KERNEL_TYPE_ALPHA 0
#define KERNEL_TYPE_BETA 1
#define KERNEL_TYPE_RC 2
#define KERNEL_TYPE_RELEASE 3

#define LENGTH_OF(x) (sizeof(x) / sizeof(x[0]))

//Typedefs

//Unsigned types
typedef unsigned long long uqword_t;
typedef unsigned long udword_t;
typedef unsigned uword_t;
typedef unsigned char ubyte_t;
typedef unsigned char uchar_t;

//Signed types
typedef   signed long long sqword_t;
typedef   signed long sdword_t;
typedef   signed sword_t;
typedef   signed char sbyte_t;
typedef   signed char schar_t;

typedef   signed long long qword_t;
typedef   signed long dword_t;
typedef   signed word_t;
typedef   signed char byte_t;
typedef   signed char char_t;

extern udword_t KERNEL_VIRTUAL_BASE;
#define KERNEL_BASE (udword_t)&KERNEL_VIRTUAL_BASE
#define PMM_BASE 0xC8000000

/* Utilities */
void kexit(int status);
void kpanic(const char* message);
void kputchar(const char c);
void kdump_useStack(uqword_t* esp);
void kdump_useRegs(uqword_t eip);
void kdumpStack(uqword_t* esp, udword_t ebp);
void itoa(qword_t number, char* str, int radix);
udword_t* getKernelVersion();

//Optimized memory functions
/*
void* kmemcpy(void* dest, const void* src, size_t num);
void* kmemmove(void* dest, const void* src, size_t num);
*/

#endif
