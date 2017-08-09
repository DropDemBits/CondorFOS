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

#include <stdint.h>
#include <kernel/stack_state.h>

#ifndef _IDT_H
#define _IDT_H

#define ISR_32_TASKGATE  0x5
#define ISR_16_TRAPGATE  0x6
#define ISR_16_INTRGATE  0x7
#define ISR_32_INTRGATE  0xE
#define ISR_32_TRAPGATE  0xF
#define ISR_ATR_PRESENT  0b10000000
#define ISR_ATR_STORESEG 0b00010000
#define ISR_ATR_RING0    0b00000000
#define ISR_ATR_RING1    0b00100000
#define ISR_ATR_RING2    0b01000000
#define ISR_ATR_RING3    0b01100000

#define IRQ0  32
#define IRQ1  IRQ0+1
#define IRQ2  IRQ0+2
#define IRQ3  IRQ0+3
#define IRQ4  IRQ0+4
#define IRQ5  IRQ0+5
#define IRQ6  IRQ0+6
#define IRQ7  IRQ0+7
#define IRQ8  IRQ0+8
#define IRQ9  IRQ0+9
#define IRQ10 IRQ0+10
#define IRQ11 IRQ0+11
#define IRQ12 IRQ0+12
#define IRQ13 IRQ0+13
#define IRQ14 IRQ0+14
#define IRQ15 IRQ0+15

typedef void(*isr_t)(stack_state_t*);

/**
 * Adds an ISR handler
 * @param int_num The interrupt to service
 * @param addr The address to the handler
 */
void idt_addISR(uint16_t int_num, isr_t addr);

/**
 * Removes an ISR handler
 * @param int_num The interrupt to detach the handler from
 */
void idt_clearISR(uint16_t int_num);

/**
 * Initializes the IDT
 * @param memory_location The location of the IDT base
 */
void idt_init(uint32_t memory_location);

#endif
