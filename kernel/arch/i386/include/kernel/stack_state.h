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

/* 
 * File:   stack_state.h
 * Author: DropDemBits <r3usrlnd@gmail.com>
 *
 * Created on April 22, 2017, 2:37 PM
 */

#include <stdint.h>

#ifndef STACK_STATE_H
#define STACK_STATE_H

// ISR Stack State
typedef struct
{
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t int_num;
    uint32_t err_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
} stack_state_t;

//Registers (for multi-threading/tasking)
typedef struct
{
    uint32_t eax; //0(eax)
    uint32_t ebx; //4(eax)
    uint32_t ecx; //8(eax)
    uint32_t edx; //12(eax)
    uint32_t esi; //16(eax)
    uint32_t edi; //20(eax)
    uint32_t esp; //24(eax)
    uint32_t ebp; //28(eax)
    uint32_t eip; //32(eax)
    uint32_t eflags; //36(eax)
    uint32_t cs; //40(eax)
    uint32_t ds; //44(eax)
    uint32_t ss; //48(eax)
    uint32_t es; //52(eax)
    uint32_t fs; //56(eax)
    uint32_t gs; //60(eax)
} registers_t;

#endif /* STACK_STATE_H */

