#include <multiboot.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <kernel/hal.h>
#include <kernel/keyboard.h>
#include <kernel/klogger.h>
#include <kernel/pmm.h>
#include <kernel/tty.h>
#include <kernel/vmm.h>
#include <kernel/ata.h>
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
    pmm_setRegion(0x0, (size_t)&kernel_end - KERNEL_BASE);

    if(info_struct->flags & 0x1)
    {
        vmm_init();
        logNorm("Initializing PMM\n");
        pmm_init(mem_size, (((udword_t)&kernel_end & 0xFFFFF000) + 0x3000 + (sizeof(MemoryRegion)*mem_pages)) & ~(0xFFF));
    }
    else kpanic("wut");

    if(info_struct->flags & 1 << 9)
    {
        logInfo("Loaded by Multiboot loader ");
        printf("%s\n", info_struct->boot_loader_name + KERNEL_BASE);
        serial_writes(COM1, (char*)info_struct->boot_loader_name + KERNEL_BASE);
        serial_writechar(COM1, '\n');
        serial_writechar(COM1, '\r');
    }
    if(info_struct->flags & 4 && info_struct->mods_count)
    {
        logInfo("Bootloader has loaded modules\n");
        printf("Number of modules: %d\n", info_struct->mods_count);
    }
    
    logNorm("Initializing Generic ATA Driver\n");
    ata_init();
    
    logNorm("Initializing Timer\n");
    hal_initTimer();
    
    logNorm("Initializing Device Controller\n");
    hal_initController();

    logNorm("Initializing keyboard\n");
    keyboard_init();
    
    //Do tests

    logNorm("Testing PMM\n");
    physical_addr_t* paddr1 = pmalloc();
    linear_addr_t* laddr = (linear_addr_t*)0xFFB00000;
    map_address(laddr, paddr1, 0x3);
    *laddr = 0xB00FBEEF;
    unmap_address(laddr);
    physical_addr_t* paddr2 = pmalloc();
    map_address(laddr, paddr2, 0x3);
    printf("PADDR1 %#lx, PADDR2 %#lx, AT ADDR: %#lx\n", paddr1, paddr2, *laddr);
    if(paddr1 != paddr2 || *laddr != 0xB00FBEEF) kpanic("PMM Test failed");
    unmap_address(laddr);

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
    logNorm("Successfully Initialized kernel\n");
    terminal_puts("\nWelcome to ");
    terminal_puts_Color("Condor!", vga_makeColor(VGA_WHITE, VGA_BLACK));
    printf(" (");
    udword_t* version = getKernelVersion();
    printf(KERNEL_VERSION_FORMATER, version[0], version[1], version[2], getKernelRelType(version[3]));
    printf(")\n");
    while(keyboard_readKey()) asm("pause");
    
    uword_t* sect = kmalloc(1024*16);
    
    for(ubyte_t device = 0; device < 4; device++) {
    	printf("\nWritting to disk %x...\nData before write: \n", device);
    	memset(sect, 0, 1024*16);
    	
    	ata_readSectors(device, 0x00, 1, sect);
    	
    	if(sect[0] == 0) continue;
    	
    	for(uword_t i = 0; i < 1024 && sect[0] != 0; i++)
   		{
    	    printf("%c%c", sect[i] & 0xFF, (sect[i] >> 8) & 0xFF);
    	}
    	
    	memset(sect, 'A', 1024*16);
    	sect[1023] = 0x2100 | 'H';
    	sect[255] = 0x2100 | 'H';
    	ata_writeSectors(ATA_DEVICE_2, 0, 0, sect);
    	
    	printf("\nReading from disk...\n");
    	memset(sect, 0x00, 1024*16);
    	ata_readSectors(device, 0x00, 1, sect);
    	for(uword_t i = 0; i < 1024 && sect[0] != 0; i++)
    	{
    	    printf("%c%c", sect[i] & 0xFF, (sect[i] >> 8) & 0xFF);
    	}
    	printf("\n");
    }
    kfree(sect);
    
    printf("\n\nwoosh\n\n");
    printf("The PMM Test doesn't work without the above two lines (including this one)");
    
    ubyte_t last_state = 0;
	
    while(1)
    {
	    ubyte_t new_char = keyboard_readKey();

        if(new_char && keyboard_getKeyState(new_char) != last_state)
        {
            if(keyboard_getChar(new_char)) printf("%c", keyboard_getChar(new_char));
            last_state = keyboard_getKeyState(new_char);
        } else last_state = 0;

        asm("pause");
    }
}
