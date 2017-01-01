#include <stdint.h>

#ifndef IDT_H
#define IDT_H

#define ISR_32_TASKGATE  0x5
#define ISR_16_TRAPGATE  0x6
#define ISR_16_INTRGATE  0x7
#define ISR_32_TRAPGATE  0xE
#define ISR_32_INTRGATE  0xF
#define ISR_ATR_PRESENT  0b10000000
#define ISR_ATR_STORESEG 0b00010000
#define ISR_ATR_RING0    0b00000000
#define ISR_ATR_RING1    0b00100000
#define ISR_ATR_RING2    0b01000000
#define ISR_ATR_RING3    0b01100000

// ISR Stacks
struct Pusha
{
    uint16_t ss;
    uint16_t gs;
    uint16_t fs;
    uint16_t es;
    uint16_t ds;
    uint16_t eax;
    uint16_t ebx;
    uint16_t ecx;
    uint16_t edx;
    uint16_t esp;
    uint16_t ebp;
    uint16_t esi;
    uint16_t edi;
};

struct CPUStack
{
    uint16_t int_num;
    uint16_t err_code;
    uint16_t eip;
    uint16_t cs;
    uint16_t eflags;
};

void idt_addISR(uint16_t int_num, uint32_t addr);
void idt_clearISR(uint16_t int_num);

void idt_init(uint32_t memory_location);

#endif
