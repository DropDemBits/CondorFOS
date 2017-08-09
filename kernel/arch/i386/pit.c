#include <stdio.h>

#include <kernel/stack_state.h>
#include <kernel/idt.h>
#include <kernel/pit.h>
#include <kernel/tasks.h>
#include <io.h>

static uint8_t counter_settings[3];

void timer_isr(stack_state_t* state)
{
#if 0
    (void) state;
    putchar('a');
#else
    process_preempt(state);
#endif
}

void pit_init()
{
    pit_createCounter(MAIN_FRQ, PIT_COUNTER0, PIT_ACCESS_MODE_LO_HI | PIT_MODE_RATE_GENERATOR);
    idt_addISR(32, timer_isr);
}

void pit_createCounter(uint32_t frequency, uint8_t counter, uint8_t mode)
{
    if(!frequency) return;
    uint32_t divisor = 1193182 / frequency;

    counter_settings[counter >> 6] = mode;
    outb(PIT_COMMAND, mode | counter << 6);

    pit_writeReload(divisor, counter);
}

void pit_writeReload(uint16_t value, uint8_t counter) {
    outb(counter, value & 0xFF);
    if(counter_settings[counter >> 6] & PIT_ACCESS_MODE_LO_HI)
        outb(counter, value >> 8);
}
