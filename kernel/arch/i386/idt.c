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
#include <kernel/idt.h>
#include <kernel/klogger.h>
#include <kernel/hal.h>
#include <kernel/pmm.h>
#include <condor.h>
#include <stdio.h>
#include <serial.h>

// ISRs
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr32();
extern void isr33();
extern void isr34();
extern void isr35();
extern void isr36();
extern void isr37();
extern void isr38();
extern void isr39();
extern void isr40();
extern void isr41();
extern void isr42();
extern void isr43();
extern void isr44();
extern void isr45();
extern void isr46();
extern void isr47();
extern void isr_handler(stack_state_t* state);

static const char* predefMSGS[] = {
    "Division by 0",
    "Debug",
    "NMI",
    "Breakpoint",
    "Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "Device not available",
    "Double Fault",
    "FPU Segment Overrun (INVALID)",
    "Invalid TSS",
    "Invalid Segment",
    "Stack Segment Fault",
    "GP Fault",
    "Page Fault",
    "Intel Reserved",
    "x87 Floating Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating Point Exception",
    "Virtual Error",
    "Intel Reserved",
    "Intel Reserved",
    "Intel Reserved",
    "Intel Reserved",
    "Intel Reserved",
    "Intel Reserved",
    "Intel Reserved",
    "Intel Reserved",
    "Intel Reserved",
    "Security Exception",
    "Intel Reserved",
    // Other
    "Invalid Excepion",
};
static isr_t isrs[256];
static isr_handler_t* irqs[NR_IRQS];
static uint32_t* descriptors;

void isr_handler(stack_state_t* state)
{
    if(isrs[state->int_num] == 0x00 && state->int_num < 32) kspanic(predefMSGS[state->int_num], state);
    else if(state->int_num < 32) {
        if(isrs[state->int_num] != 0x00) {
            isrs[state->int_num](state);
        }
    } else if(state->int_num >= IRQ0 && state->int_num < NR_IRQS + IRQ0) {
        //printf("isr_handler\n");
        //asm("xchg %bx, %bx");
        isr_handler_t* action = irqs[state->int_num-IRQ0];

        while(action != POISON_NULL) {
            irqreturn_t ret_val = action->handler(state);

            if(ret_val == HANDLED) {
                ic_ack(state->int_num-IRQ0);
                break;
            }

            action = action->next;
        }
    }
}

static void idt_registerInterrupt(uint16_t int_num, uint32_t func_addr, uint16_t gdt_selector, uint8_t type_attrib)
{
    if(int_num > 255) return;
    int_num *= 2;
    uint32_t desc_lower =(func_addr & 0xFFFF) | (gdt_selector << 16);
    uint32_t desc_higher = 0x00 | (type_attrib << 8) | (func_addr & 0xFFFF0000);
    descriptors[int_num+0] = desc_lower;
    descriptors[int_num+1] = desc_higher;
}

void idt_addISR(uint16_t int_num, isr_t handler)
{
    if(handler != POISON_NULL) {
        isrs[int_num] = handler;
    }
}

void idt_addIRQ_ISR(uint16_t int_num, irq_t handler)
{
    if(int_num >= NR_IRQS || handler == POISON_NULL) return;
    hal_disableInterrupts();
    printf("idt_addIRQ_ISR: %d, %lx\n", int_num, handler);
    isr_handler_t* action = kmalloc(sizeof(isr_handler_t));
    action->next = POISON_NULL;
    action->handler = handler;

    if(irqs[int_num] == POISON_NULL) {
        irqs[int_num] = action;
    }
    else {
        isr_handler_t* handler = irqs[int_num];

        while(handler->next != POISON_NULL) {
            handler = handler->next;
        }

        handler->next = action;
    }
    hal_restoreInterrupts();
}

void idt_init(uint32_t memory_location)
{
    descriptors = (uint32_t*)memory_location;

    idt_registerInterrupt( 0, (uint32_t)isr0,  0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt( 1, (uint32_t)isr1,  0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt( 2, (uint32_t)isr2,  0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt( 3, (uint32_t)isr3,  0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt( 4, (uint32_t)isr4,  0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt( 5, (uint32_t)isr5,  0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt( 6, (uint32_t)isr6,  0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt( 7, (uint32_t)isr7,  0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt( 8, (uint32_t)isr8,  0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt( 9, (uint32_t)isr9,  0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(10, (uint32_t)isr10, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(11, (uint32_t)isr11, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(12, (uint32_t)isr12, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(13, (uint32_t)isr13, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(14, (uint32_t)isr14, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(15, (uint32_t)isr15, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(16, (uint32_t)isr16, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(17, (uint32_t)isr17, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(18, (uint32_t)isr18, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(19, (uint32_t)isr19, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(20, (uint32_t)isr20, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(21, (uint32_t)isr21, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(22, (uint32_t)isr22, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(23, (uint32_t)isr23, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(24, (uint32_t)isr24, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(25, (uint32_t)isr25, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(26, (uint32_t)isr26, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(27, (uint32_t)isr27, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(28, (uint32_t)isr28, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(29, (uint32_t)isr29, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(30, (uint32_t)isr30, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(31, (uint32_t)isr31, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    hal_initPIC(IRQ0);
    //Hardware interrupts
    idt_registerInterrupt(IRQ0,  (uint32_t)isr32, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(IRQ1,  (uint32_t)isr33, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(IRQ2,  (uint32_t)isr34, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(IRQ3,  (uint32_t)isr35, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(IRQ4,  (uint32_t)isr36, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(IRQ5,  (uint32_t)isr37, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(IRQ6,  (uint32_t)isr38, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(IRQ7,  (uint32_t)isr39, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(IRQ8,  (uint32_t)isr40, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(IRQ9,  (uint32_t)isr41, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(IRQ10, (uint32_t)isr42, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(IRQ11, (uint32_t)isr43, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(IRQ12, (uint32_t)isr44, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(IRQ13, (uint32_t)isr45, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(IRQ14, (uint32_t)isr46, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(IRQ15, (uint32_t)isr47, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    //Clear ISRs
    for(size_t i = 0; i < NR_IRQS; i++) {
        irqs[i] = POISON_NULL;
    }
    //Mask all but irqs 0, 1, 2, 12, 14, and 15
    ic_unmaskIRQ(0);
    ic_unmaskIRQ(1);
    ic_unmaskIRQ(2);
    ic_maskIRQ(3);
    ic_maskIRQ(4);
    ic_maskIRQ(5);
    ic_maskIRQ(6);
    ic_maskIRQ(7);
    ic_maskIRQ(8);
    ic_maskIRQ(9);
    ic_maskIRQ(10);
    ic_maskIRQ(11);
    ic_unmaskIRQ(12);
    ic_maskIRQ(13);
    ic_unmaskIRQ(14);
    ic_unmaskIRQ(15);
}
