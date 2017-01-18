#include <multiboot.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <kernel/keyboard.h>
#include <kernel/klogger.h>
#include <kernel/pmm.h>
#include <kernel/timer_hal.h>
#include <kernel/tty.h>
#include <condor.h>
#include <serial.h>

extern uint32_t kernel_end;

static char* mmap_types[6] = {"INV", "Available", "Reserved", "ACPI Reclaimable", "NVS", "BadRam"};

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
    
    if(info_struct->flags & (1 << 6))
    {
        printf("Memory Size: Low: %ldkb, %ldkb, Total: %ldkb\n", info_struct->mem_lower, info_struct->mem_upper, info_struct->mem_lower+(info_struct->mem_upper));
         
        pmm_setRegionBase((((physical_addr_t)&kernel_end) & 0xFFFFF000) + 0x1000);
        multiboot_memory_map_t* mmap = (multiboot_memory_map_t*) info_struct->mmap_addr;
        
        while(((udword_t)mmap) < info_struct->mmap_addr + info_struct->mmap_length)
        {
            //Process MMAP
            
            if(mmap->type > MULTIBOOT_MEMORY_AVAILABLE) {
                pmm_setRegion((udword_t)mmap->addr, (udword_t)mmap->len);
            }
            printf("base_addr = 0x%lx, length = 0x%lx, %s\n", (udword_t)mmap->addr, (udword_t)mmap->len, mmap_types[mmap->type]);
            
            mmap = (multiboot_memory_map_t*) ((unsigned int)mmap + mmap->size + sizeof(mmap->size));
        }
    }
    
    if(info_struct->flags & 0x1)
    {
        pmm_init(1024+info_struct->mem_lower+(info_struct->mem_upper*64), (((physical_addr_t)&kernel_end) & 0xFFFFF000)+0x2000);
        printf("Addrs: %lx, %lx, %lx\n", (physical_addr_t)&kernel_end, (((physical_addr_t)&kernel_end) & 0xFFFFF000) + 0x1000, (((physical_addr_t)&kernel_end) & 0xFFFFF000) + 0x2000);
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
}

void kmain()
{
    serial_writes(COM1, "all\n");
    logNorm("Successfully booted kernel\n");
    terminal_puts("\nWelcome to ");
    terminal_puts_Color("CondorFOS!\n", vga_makeColor(VGA_WHITE, VGA_BLACK));
    
    for(;;) asm("hlt");
}
