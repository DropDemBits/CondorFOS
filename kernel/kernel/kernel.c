#include <multiboot.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

void kputchar(const char c)
{
    terminal_putchar(c);
}

void kinit(multiboot_info_t *info_struct, uint32_t magic)
{
    serial_init(COM1, 0x0003);
    serial_writes(COM1, "Successfully initialized serial port");
    terminal_init();
    logNorm("Initialized tty\n");

    if(magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        printf("EAX is %lx\n", magic);
        printf("EBX is %lx\n", info_struct);
        kpanic("Not loaded by a multiboot bootloader");
    }

    printf("Loaded by bootloader %s\n", info_struct->boot_loader_name);
    if(info_struct->flags & 4)
    {
        printf("Bootloader has loaded modules:\n");
        printf("Number of modules: %d\n", info_struct->mods_count);
    }

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
    serial_writes(COM1, "all\n");
    logNorm("Successfully booted kernel\n");
    terminal_puts("\nWelcome to ");
    terminal_puts_Color("CondorFOS!\n", vga_makeColor(VGA_WHITE, VGA_BLACK));
    printf("%s\n", "aasd");
    printf("%x\n", 0xFAD5);
    printf("%lx\n", 0xFEEDBEEF);
    
    for(condor_ubyte_t derp = 20; derp != 0xFF; derp--)
    {
        printf("Hello derp: %d\n", derp);
    }
    for(;;) asm("hlt");
}
