#include <stdio.h>

#include <kernel/stack_state.h>
#include <kernel/idt.h>
#include <kernel/pit.h>
#include <kernel/tasks.h>
#include <io.h>

static volatile uint32_t _timer_ticks;
static volatile uint32_t _timer_millis;

void timer_isr(stack_state_t* state)
{
    _timer_ticks++;
    if(_timer_ticks % MILLI_INTERVAL) {
        _timer_millis++;
        process_preempt(state);
    }
}

void pit_init()
{
    pit_createCounter(MAIN_FRQ, PIT_COUNTER0, 0x34);
    pit_createCounter(0x1, PIT_COUNTER2, 0x36);
    idt_addISR(32, (uint32_t)timer_isr);
}

void pit_createCounter(uint32_t frequencey, uint8_t counter, uint8_t mode)
{
    if(!frequencey) return;
    uint32_t divisor = 1193182 / frequencey;

    outb(PIT_COMMAND, mode | counter << 6);

    outb(counter, (divisor >> 0) & 0xFF);
    outb(counter, (divisor >> 8) & 0xFF);
}

uint32_t pit_getTicks()
{
    return _timer_ticks;
}

uint32_t pit_getMillis()
{
    return _timer_ticks / MILLI_INTERVAL;
}

void pit_tsleep(uint32_t ticks)
{
    //TODO: Make thread safe
    asm("pushf\n\tsti");
    volatile uint32_t end_ticks = _timer_ticks+ticks;
    while(_timer_ticks <= end_ticks) asm("hlt");
    asm("popf");
    return;
}

void pit_sleep(uint32_t millis)
{
    //TODO: Make thread safe
    asm("pushf\n\tsti");
    volatile uint32_t end_millis = _timer_millis+millis;
    while(_timer_millis <= end_millis) asm("hlt");
    asm("popf");
    return;
}
