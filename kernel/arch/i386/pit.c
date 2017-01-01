#include <kernel/idt.h>
#include <kernel/pit.h>
#include <io.h>

static uint32_t _timer_ticks;
static uint32_t _timer_secs;

void timer_isr()
{
    _timer_ticks++;
    if(_timer_ticks % 72 == 0) _timer_secs++;
}

void pit_init()
{
    pit_createCounter(1, PIT_COUNTER0, 0x36);
    idt_addISR(32, (uint32_t)timer_isr);
}

void pit_createCounter(uint32_t frequencey, uint8_t counter, uint8_t mode)
{
    if(!frequencey) return;
    uint32_t divisor = 1193180;

    outb(PIT_COMMAND, mode);

    outb(counter, (divisor >> 0) & 0xFF);
    outb(counter, (divisor >> 8) & 0xFF);
}

uint32_t pit_getTicks()
{
    return _timer_ticks;
}

uint32_t pit_getSeconds()
{
    return _timer_ticks % 36;
}

void pit_sleep(uint32_t ticks)
{
    uint32_t start_ticks = _timer_ticks;
    while(_timer_ticks >= (ticks+start_ticks)) asm("hlt");
    return;
}

void timer_init()
{
    pit_createCounter(1, PIT_COUNTER0, 0x36);
    idt_addISR(32, (uint32_t)timer_isr);
}

void timer_createCounter(uint32_t frequencey, uint8_t counter, uint8_t mode)
{
    if(!frequencey) return;
    uint32_t divisor = 1193180;

    outb(PIT_COMMAND, mode);

    outb(counter, (divisor >> 0) & 0xFF);
    outb(counter, (divisor >> 8) & 0xFF);
}

uint32_t get_timer_ticks()
{
    return _timer_ticks;
}

uint32_t get_timer_seconds()
{
    return _timer_secs;
}

void sleep(uint32_t ticks)
{
    uint32_t start_ticks = _timer_ticks;
    while((ticks+start_ticks) < _timer_ticks) asm("hlt");
    return;
}
