#include <stdint.h>

#ifndef IDT_H
#define IDT_H

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

// ISR Stack State
typedef struct
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t edx;
    uint32_t ecx;
    uint32_t ebx;
    uint32_t eax;
    uint32_t int_num;
    uint32_t err_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
} stack_state_t;

/**
 * Adds an ISR handler
 * @param int_num The interrupt to service
 * @param addr The address to the handler
 */
void idt_addISR(uint16_t int_num, uint32_t addr);

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
