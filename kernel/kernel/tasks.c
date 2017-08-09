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
#include <kernel/stack_state.h>
#include <kernel/vmm.h>
#include <kernel/tty.h>
#include <kernel/vga.h>
#include <condor.h>

extern void idle_process();

static uint32_t pid_counter = 0;
process_t* p_last = NULL;
process_t* p_current = NULL;

process_t* process_create(void (*entry_point)(void))
{
    hal_disableInterrupts();
    process_t* proc = kmalloc(sizeof(process_t));
    if(proc == NULL)
        return proc;
    memset(proc, 0x00, sizeof(process_t));

    registers_t* regs = kmalloc(sizeof(registers_t));
    if(regs == NULL) {
        kfree(proc);
        return NULL;
    }
    memset(regs, 0x00, sizeof(registers_t));

    proc->regs = regs;
    proc->pid = pid_counter++;
    proc->page_base = vmm_get_current_page_base();


    // TODO: Move somewhere else
    linear_addr_t* stack_base = vmalloc(1);
    physical_addr_t* stack_phybase = pmalloc();
    if(vmm_map_address(stack_base, stack_phybase, PAGE_PRESENT | PAGE_RW) == 2) {
        kpanic("VADDR Alloc Failed (process_create)");
    }

    proc->regs->eip = (uint32_t)entry_point;
    proc->regs->esp = (uint32_t)stack_base+0xFFC;
    proc->regs->cs = 0x08;
    proc->regs->ds = 0x10;
    proc->regs->es = 0x10;
    proc->regs->fs = 0x10;
    proc->regs->gs = 0x10;
    proc->regs->ss = 0x10;
    proc->regs->eflags = 0x202;

    if(p_current == NULL) {
        p_current = proc;
    }
    else if(p_last == NULL) {
        p_current->next = proc;
        p_last = proc;
    }
    else {
        p_last->next = proc;
        p_last = proc;
    }

    hal_enableInterrupts();
    return proc;
}

void swap_registers(registers_t* src, registers_t* dest)
{
    memcpy(dest, src, sizeof(registers_t));
}

void process_preempt(stack_state_t* state)
{
    uint32_t int_num = 0;

    if(p_current == NULL) return;


    if(p_current->current_state != INITIALIZED) swap_registers(state, p_current->regs);
    else p_current->current_state = RUNNING;

    int_num = state->int_num;
    if(p_current->next == NULL) goto skip;
    //if(p_current->next->pid == 0) p_last->next = p_current;

    swap_registers(state, p_current->regs);
    if(p_last != NULL) p_last->next = p_current;
    p_last = p_current;
    p_current = p_last->next;
    p_last->next = NULL;

    skip:
    swap_registers(p_current->regs, state);
    state->int_num = int_num;


    terminal_storePosition();
    terminal_set_shouldUpdateCursor(0);
    terminal_moveCursor(0, 0);
    uint16_t old_color = terminal_getColor();
    terminal_setColor(VGA_WHITE, VGA_BLACK);

    printf("Current PID(%d): (EIP: %#lx)\n", p_current->pid, p_current->regs->eip);

    terminal_setColor(old_color & 0xFF, old_color >> 8);
    terminal_set_shouldUpdateCursor(1);
    terminal_restorePosition();
}
