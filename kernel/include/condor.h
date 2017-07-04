/*
 * Copyright (C) 2017 DropDemBits <r3usrlnd@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <kernel/stack_state.h>
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

/* Utilities */
void kexit(int status);
void kpanic(const char* message);
void kspanic(const char* message, stack_state_t* state);
void kputchar(const char c);
void kdump_useStack(stack_state_t* esp);
void kdump_useRegs(uqword_t eip);
void kdumpStack(uqword_t* esp, udword_t ebp);
udword_t* getKernelVersion();

//Optimized memory functions
/*
void* kmemcpy(void* dest, const void* src, size_t num);
void* kmemmove(void* dest, const void* src, size_t num);
*/
void* kmemset(void* base, const int c, size_t length);


#endif
