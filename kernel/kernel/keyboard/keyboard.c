#include <kernel/keyboard.h>
#include <kernel/idt.h>
#include <kernel/timer_hal.h>
#include <kernel/tty.h>
#include <condor.h>

void keyboard_isr()
{
    char keycode = inb(PS2_DATA);
    terminal_putchar(keyboard_getChar(keycode));
    return;
}

void keyboard_init(void)
{
    idt_addISR(33, (udword_t) keyboard_isr);
}

char keyboard_getChar(ubyte_t keycode)
{
    return keycode-keycode;
}
