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
 * File:   threads.h
 * Author: DropDemBits <r3usrlnd@gmail.com>
 *
 * Created on April 22, 2017, 2:31 PM
 */

#include <kernel/stack_state.h>
#include <kernel/addrs.h>

#ifndef THREADS_H
#define THREADS_H

#define MAX_THREAD_COUNT 255
#define MAX_PROCESS_TIMESLICE 1
#define MAX_IDLE_SLICE 1

typedef enum {
    INITIALIZED,
    RUNNING,
    SLEEPING,
    BLOCKED,
    KERNEL_REQUEST,
} TaskState;

typedef struct Process {
    struct Process* next;
    TaskState current_state;
    int pid;
    int timeslice;
    physical_addr_t page_base;
    registers_t* regs;
} process_t;

extern void init_regs(registers_t* regs, linear_addr_t entry_point);
extern void swap_registers(registers_t* src_regs, registers_t* dest_regs);

process_t* process_create();
void process_preempt(stack_state_t* state);

#endif /* THREADS_H */
