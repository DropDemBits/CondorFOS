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
    proc->timeslice = MAX_PROCESS_TIMESLICE;
    proc->pid = pid_counter++;
    proc->page_base = vmm_get_current_page_base();
    
    
    // TODO: Move somewhere else
    linear_addr_t* stack_base = vmalloc(1);
    vmm_map_address(stack_base, pmalloc(1), PAGE_PRESENT | PAGE_RW);
    
    proc->regs->eip = (uint32_t)entry_point;
    proc->regs->esp = (uint32_t)stack_base;
    proc->regs->cs = 0x08;
    proc->regs->ds = 0x10;
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
    dest->eax = src->eax;
    dest->ebx = src->ebx;
    dest->ecx = src->ecx;
    dest->edx = src->edx;
    dest->edi = src->esi;
    dest->edi = src->edi;
    dest->esp = src->esp;
    dest->ebp = src->ebp;
    dest->eip = src->eip;
    dest->eflags = src->eflags;
    dest->cs = src->cs;
    dest->ds = src->ds;
    dest->es = src->es;
    dest->fs = src->fs;
    dest->gs = src->gs;
    dest->ss = src->ss;
}

void process_preempt(stack_state_t* state)
{
    if(p_current == NULL) return;
    
    if((p_current->regs->eip & 0xC0000000) ^ 0xC0000000) {
        state->err_code = p_current->regs->eip;
        state->eax = p_current->pid;
        kspanic("Malformed EIP", state);
    }
    
    if(p_current->current_state == INITIALIZED) {
        swap_registers(p_current->regs, state);
        p_current->current_state = RUNNING;
    }
    else if(p_current->next == NULL) p_current->timeslice = MAX_PROCESS_TIMESLICE;
    else if(--p_current->timeslice <= 0) {
        swap_registers(state, p_current->regs);
        if(p_current->pid != 0) p_current->timeslice = MAX_PROCESS_TIMESLICE;
        else p_current->timeslice = MAX_IDLE_SLICE;
        p_last->next = p_current;
        p_last = p_current;
        p_current = p_last->next;
        p_last->next = NULL;
        swap_registers(p_current->regs, state);
    }
    
    
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
