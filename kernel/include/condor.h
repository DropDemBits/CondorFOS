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

//Typedefs

//Unsigned types
typedef unsigned long long condor_uqword_t;
typedef unsigned long condor_udword_t;
typedef unsigned condor_uword_t;
typedef unsigned char condor_ubyte_t;
typedef unsigned char condor_uchar_t;

//Signed types
typedef   signed long long condor_sqword_t;
typedef   signed long condor_sdword_t;
typedef   signed condor_sword_t;
typedef   signed char condor_sbyte_t;
typedef   signed char condor_schar_t;

typedef   signed long long condor_qword_t;
typedef   signed long condor_dword_t;
typedef   signed condor_word_t;
typedef   signed char condor_byte_t;
typedef   signed char condor_char_t;

/* Bindings for libk */
void kexit(int status);
void kpanic(const char* message);
void kputchar(const char c);

#endif
