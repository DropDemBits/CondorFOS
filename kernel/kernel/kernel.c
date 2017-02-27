#include <multiboot.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <kernel/keyboard.h>
#include <kernel/klogger.h>
#include <kernel/pmm.h>
#include <kernel/timer_hal.h>
#include <kernel/tty.h>
#include <kernel/vmm.h>
#include <condor.h>
#include <serial.h>

extern udword_t kernel_start;
extern udword_t kernel_end;

static char* mmap_types[6] = {"INV", "Available", "Reserved", "ACPI Reclaimable", "NVS", "BadRam"};

void kinit(multiboot_info_t *info_struct, uint32_t magic)
{
    serial_init(COM1, 0x0003);
    serial_writes(COM1, "Successfully initialized serial port COM1\n");
    terminal_init();
    logNorm("Initialized terminal\n");

    if(magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        printf("EAX is %lx\n", magic);
        printf("EBX is %lx\n", info_struct);
        kpanic("Not loaded by a multiboot bootloader");
    }
    
    printf("Kernel (Start: %lx, End: %lx)\n", &kernel_start, &kernel_end);
    
    int mem_pages = 1;
    uqword_t mem_size = 0;
    if(info_struct->flags & (1 << 6))
    {
        printf("Memory Size: Low: %ldkb, %ldkb, Total: %ldkb\n", info_struct->mem_lower, info_struct->mem_upper, info_struct->mem_lower+(info_struct->mem_upper));
        
        pmm_setRegionBase(((udword_t)&kernel_end & 0xFFFFF000) + 0x1000);
        multiboot_memory_map_t* mmap = (multiboot_memory_map_t*) (info_struct->mmap_addr | KERNEL_BASE);
        
        while(((udword_t)mmap) < (info_struct->mmap_addr | KERNEL_BASE) + info_struct->mmap_length)
        {
            mem_pages++;
            
            //Process MMAP
            
            if(mmap->type > MULTIBOOT_MEMORY_AVAILABLE)
            {
                pmm_setRegion((udword_t)mmap->addr, (udword_t)mmap->len);
            }
            else if(mmap->type == MULTIBOOT_MEMORY_AVAILABLE)
                mem_size = mmap->addr + mmap->len;
            printf("base_addr = 0x%lx, length = 0x%lx, %s\n", (udword_t)mmap->addr, (udword_t)mmap->len, mmap_types[mmap->type]);
            
            mmap = (multiboot_memory_map_t*) ((unsigned int)mmap + mmap->size + sizeof(mmap->size));
        }
    }
    if(!mem_size) kpanic("Unable to get memory size");
    
    //Remove regions already occupied/mapped
    pmm_setRegion(0x0, 0x00800000);
    
    if(info_struct->flags & 0x1)
    {
        vmm_init();
        logNorm("Initializing PMM\n");
        pmm_init(mem_size, (((udword_t)&kernel_end & 0xFFFFF000) + 0x2000 + (sizeof(MemoryRegion)*mem_pages)) & ~(0xFFF));
    }
    else kpanic("wut");
    
    
    if(info_struct->flags & 1 << 9)
    {
        logInfo("Loaded by Multiboot loader ");
        printf("%s\n", info_struct->boot_loader_name + KERNEL_BASE);
        serial_writes(COM1, (char*)info_struct->boot_loader_name + KERNEL_BASE);
        serial_writechar(COM1, '\n');
    }
    if(info_struct->flags & 4 && info_struct->mods_count)
    {
        logInfo("Bootloader has loaded modules\n");
        printf("Number of modules: %d\n", info_struct->mods_count);
    }

    logNorm("Initializing keyboard\n");
    keyboard_init();

    logNorm("Initializing Timer\n");
    timer_init();
    
    //Initialization done, Enable interrupts
    asm("sti");
}

extern physical_addr_t* stack_base;
extern size_t stack_offset;

static char* getKernelRelType(udword_t type)
{
    switch(type)
    {
    case KERNEL_TYPE_ALPHA:   return KERNEL_ALPHA_STR;
    case KERNEL_TYPE_BETA:    return KERNEL_BETA_STR;
    case KERNEL_TYPE_RC:      return KERNEL_RC_STR;
    case KERNEL_TYPE_RELEASE: return KERNEL_RELEASE_STR;
    default:                  return "\0";
    }
}

void kmain()
{
    serial_writes(COM1, "all\n");
    logNorm("Successfully Initialized kernel\n");
    terminal_puts("\nWelcome to ");
    terminal_puts_Color("Condor!", vga_makeColor(VGA_WHITE, VGA_BLACK));
    printf(" (");
    udword_t* version = getKernelVersion();
    printf(KERNEL_VERSION_FORMATER, version[0], version[1], version[2], getKernelRelType(version[3]));
    printf(")\n");
    
    for(;;) asm("hlt");
}
