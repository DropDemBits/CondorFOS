#include <kernel/keyboard.h>
#include <kernel/klogger.h>
#include <kernel/timer_hal.h>
#include <kernel/tty.h>
#include <condor.h>
#include <serial.h>

void kexit(int status)
{
    if(!status)
        kpanic("Placeholder");
    else
        kpanic("Abnormal exit");
}

void kpanic(const char* message)
{
    logFErr(message);
    asm("cli");
    for(;;) asm("hlt");
}

void kinit()
{
    terminal_init();
    logNorm("Initialized tty\n");
    serial_init(COM1, 0x0003);
    logNorm("Initializing keyboard\n");
    keyboard_init();
    logNorm("Done Initializing keyboard\n");

    logNorm("Initializing keyboard\n");
    keyboard_init();
    logNorm("Done Initializing keyboard\n");

    logNorm("Initializing Timer\n");
    timer_init();
    logNorm("Done Initializing Timer\n");

    //Initialization done, Enable interrupts
    asm("sti");
    logNorm("Done kinit()\n");
}

void kmain()
{
    serial_writes(COM1, "all\n", 4);
    logFine("Log Test\n");
    logDebg("Log Test\n");
    logNorm("Log Test\n");
    logWarn("Log Test\n");
    logErro("Log Test\n");
    logFErr("Log Test\n");
    logNorm("Successfully booted kernel\n");
    terminal_puts("\nWelcome to ");
    terminal_puts_Color("CondorFOS!\n", vga_makeColor(VGA_WHITE, VGA_BLACK));

    int time = 0;

    for(;;) {
        time = 10-get_timer_seconds();
        terminal_moveCursor(37, 12);
        if(time > 9) {
            terminal_puts("10");
        }
        else if(time == -1) {
            terminal_clear();
            break;
        }
        else {
            terminal_putchar(' ');
            terminal_putchar(time+'0');
        }
        asm("hlt");
    }

    uint8_t clr = 1;
    for(;;)
    {
        terminal_setColor(clr, VGA_BLACK);
        terminal_clear();
        terminal_moveCursor(39-7, 12);
        terminal_puts("Happy New YEAR!");
        asm("hlt");
        terminal_setColor(VGA_BLACK, clr);
        terminal_clear();
        terminal_moveCursor(39-7, 12);
        terminal_puts("Happy New YEAR!");
        asm("hlt");
        sleep(288);
        sleep(288);
        if(++clr > 15) clr = 1;
    }
}
