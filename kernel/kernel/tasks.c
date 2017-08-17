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

#include <string.h>
#include <stdio.h>

#include <kernel/hal.h>
#include <kernel/pmm.h>
#include <kernel/tasks.h>
#include <kernel/scheduler.h>
#include <kernel/vmm.h>
#include <kernel/vga.h>
#include <condor.h>

extern void idle_process();

static uint32_t pid_counter = 0;

process_t* process_create(void *entry_point, bool is_usermode)
{
    process_t* proc = kmalloc(sizeof(process_t));
    if(proc == NULL)
        return POISON_NULL;
    memset(proc, 0x00, sizeof(process_t));

    registers_t* regs = kmalloc(sizeof(registers_t));
    if(regs == NULL) {
        kfree(proc);
        return POISON_NULL;
    }
    memset(regs, 0x00, sizeof(registers_t));

	proc->next = POISON_NULL;
    proc->regs = regs;
    proc->pid = pid_counter++;
	proc->is_usermode = is_usermode;
    proc->page_base = vmm_get_current_page_base();

    // TODO: Move somewhere else

    linear_addr_t* stack_base = vmalloc(1);
    linear_addr_t* kstack_base = vmalloc(1);

    if(is_usermode) {
        if(vmm_map_address(stack_base,
                           pmalloc(),
                           PAGE_PRESENT | PAGE_RW | PAGE_USER) == 2 ||
           vmm_map_address(kstack_base,
                           pmalloc(),
                           PAGE_PRESENT | PAGE_RW) == 2)
            kpanic("VADDR Alloc Failed (process_create)");
    }
    else {
        if(vmm_map_address(stack_base,
                           pmalloc(),
                           PAGE_PRESENT | PAGE_RW) == 2)
            kpanic("VADDR Alloc Failed (process_create)");
        vfree(kstack_base, 1);
    }

    ((stack_state_t*) proc->regs)->eip = (uint32_t)entry_point;
    ((stack_state_t*) proc->regs)->esp = (uint32_t)stack_base+0xFFC;
	if(is_usermode) {
    	((stack_state_t*) proc->regs)->cs = (0x3 << 3) | 0b11;
    	((stack_state_t*) proc->regs)->ds = (0x4 << 3) | 0b11;
    	((stack_state_t*) proc->regs)->es = (0x4 << 3) | 0b11;
    	((stack_state_t*) proc->regs)->fs = (0x4 << 3) | 0b11;
    	((stack_state_t*) proc->regs)->gs = (0x4 << 3) | 0b11;
    	((stack_state_t*) proc->regs)->ss = (0x4 << 3) | 0b11;
        proc->regs->kernel_esp = (physical_addr_t)kstack_base;
	}
	else {
		((stack_state_t*) proc->regs)->cs = (0x1 << 3) | 0b00;
    	((stack_state_t*) proc->regs)->ds = (0x2 << 3) | 0b00;
    	((stack_state_t*) proc->regs)->es = (0x2 << 3) | 0b00;
    	((stack_state_t*) proc->regs)->fs = (0x2 << 3) | 0b00;
    	((stack_state_t*) proc->regs)->gs = (0x2 << 3) | 0b00;
    	((stack_state_t*) proc->regs)->ss = (0x2 << 3) | 0b00;
        proc->regs->kernel_esp = (physical_addr_t)stack_base;
	}
    ((stack_state_t*) proc->regs)->eflags = 0x202;

    schedule_process(proc);

    return proc;
}
