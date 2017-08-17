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

#ifndef _TYPES_H_
#define _TYPES_H_

#ifndef POISON_NULL
#define POISON_NULL (void*)0xfeeeeb1e
#endif

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

#include <kernel/addrs.h>
#include <kernel/stack_state.h>

#endif /* _TYPES_H_ */
