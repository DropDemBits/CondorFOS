#include <kernel/klogger.h>
#include <kernel/tty.h>
#include <serial.h>

void kinit()
{
    terminal_init();
    serial_init(COM1, 0x0003);
}

void kmain()
{
    serial_writes(COM1, "all\n", 4);
    logFine("Log Test\n", 9);
    logDebg("Log Test\n", 9);
    logNorm("Log Test\n", 9);
    logWarn("Log Test\n", 9);
    logErro("Log Test\n", 9);
    logFErr("Log Test\n", 9);
    logNorm("Successfully booted kernel\n", 27);
    terminal_puts("\nWelcome to ", 12);
    terminal_puts_Color("CondorFOS!\n", 11, vga_makeColor(VGA_WHITE, VGA_BLACK));
    for(;;);
}
