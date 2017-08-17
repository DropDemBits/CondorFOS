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
#include <kernel/scheduler.h>
#include <kernel/stack_state.h>
#include <kernel/vmm.h>
#include <kernel/tty.h>
#include <kernel/vga.h>
#include <condor.h>

extern void tss_set_esp0(uint32_t kernel_esp);

process_t* p_last = POISON_NULL;
process_t* p_current = POISON_NULL;
bool is_schedueling_proc = false;

void schedule_process(process_t* process)
{
    hal_disableInterrupts();
    is_schedueling_proc = true;

    if(p_current == POISON_NULL) {
        p_current = process;
    }
    else if(p_last == POISON_NULL) {
        p_current->next = process;
        p_last = process;
    }
    else {
        p_last->next = process;
        p_last = process;
    }

    is_schedueling_proc = false;
    hal_restoreInterrupts();
}

void scheduler_preempt(stack_state_t* state)
{
    uint32_t int_num = 0;

    if(p_current == POISON_NULL || is_schedueling_proc) return;

    int_num = state->int_num;
    if(p_current->next == POISON_NULL) goto skip;

    // Save Registers
    if(p_current->current_state != INITIALIZED) memcpy(p_current->regs, state, sizeof(stack_state_t));
    else p_current->current_state = RUNNING;
    if(p_last != POISON_NULL) p_last->next = p_current;
    p_last = p_current;
    p_current = p_last->next;
    p_last->next = POISON_NULL;
    tss_set_esp0(p_current->regs->kernel_esp);

    skip:
    // Restore registers
    memcpy(state, p_current->regs, sizeof(stack_state_t));
    state->int_num = int_num;

    terminal_storePosition();
    terminal_set_shouldUpdateCursor(0);
    terminal_moveCursor(0, 0);
    uint16_t old_color = terminal_getColor();
    terminal_setColor(VGA_WHITE, VGA_BLACK);

    printf("Current PID(%d): (EIP: %#lx)\n", p_current->pid, ((stack_state_t*) p_current->regs)->eip);

    terminal_setColor(old_color & 0xFF, old_color >> 8);
    terminal_set_shouldUpdateCursor(1);
    terminal_restorePosition();

    //asm("xchg %bx, %bx");
}
