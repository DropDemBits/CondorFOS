/*
 * Copyright (C) 2017 DropDemBits <r3usrlnd@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <multiboot.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <kernel/hal.h>
#include <kernel/keyboard.h>
#include <kernel/klogger.h>
#include <kernel/pmm.h>
#include <kernel/tty.h>
#include <kernel/vga.h>
#include <kernel/tasks.h>
#include <kernel/vmm.h>
#include <kernel/ata.h>
#include <condor.h>
#include <serial.h>

extern udword_t kernel_start;
extern udword_t kernel_end;
extern int kmain();
extern int yeet();
extern int zeet();

extern void idle_process();

static char* mmap_types[6] = {"INV", "Available", "Reserved", "ACPI Reclaimable", "NVS", "BadRam"};

void kinit(multiboot_info_t *info_struct, uint32_t magic)
{
    serial_init(COM1, 0x0003);
    serial_writes(COM1, "Successfully initialized serial port COM1\n");
    terminal_init();
    logNorm("Initialized terminal\n");

    if(magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        printf("EAX is %lx\n", magic);
        printf("EBX is %lx\n", info_struct);
        kpanic("Not loaded by a multiboot bootloader");
    }

    printf("Kernel (Start: %lx, End: %lx), MB_INFO: %#lx\n", &kernel_start, &kernel_end, vmm_get_physical_addr((linear_addr_t*)info_struct));

    if(info_struct->flags & MULTIBOOT_INFO_MEMORY) {
        printf("Memory Size: Low: %ldkb, %ldkb, Total: %ldMiB\n", info_struct->mem_lower, info_struct->mem_upper, (info_struct->mem_lower+info_struct->mem_upper)/1024);
    }

    printf("Modules: %#lx\n", info_struct->framebuffer_addr);
    logNorm("Initializing VMM\n");
    vmm_init();

    if(info_struct->flags & MULTIBOOT_INFO_MEM_MAP) {
        multiboot_memory_map_t* mmap = (multiboot_memory_map_t*) (info_struct->mmap_addr | KERNEL_BASE);
        logNorm("Initializing PMM\n");
        pmm_init(mmap, info_struct->mmap_length);

        //Remove regions already occupied/mapped
        pmm_set_region((linear_addr_t)&kernel_start - KERNEL_BASE, (size_t)&kernel_end - KERNEL_BASE);

        while(((udword_t)mmap) < (info_struct->mmap_addr | KERNEL_BASE) + info_struct->mmap_length) {
            //Print MMAP
            printf("base_addr = 0x%lx, length = 0x%lx, %s\n", (udword_t)mmap->addr, (udword_t)mmap->len, mmap_types[mmap->type]);
            mmap = (multiboot_memory_map_t*) ((unsigned int)mmap + mmap->size + sizeof(mmap->size));
        }
    }

    logNorm("Initializing Core VADDM\n");
    vaddm_init();

    if(info_struct->flags & MULTIBOOT_INFO_BOOT_LOADER_NAME) {
        logInfo("Loaded by Multiboot loader ");
        printf("%s\n", info_struct->boot_loader_name + KERNEL_BASE);
        serial_writes(COM1, (char*)info_struct->boot_loader_name + KERNEL_BASE);
        serial_writechar(COM1, '\n');
        serial_writechar(COM1, '\r');
    }
    if(info_struct->flags & MULTIBOOT_INFO_MODS && info_struct->mods_count) {
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
#ifndef _DO_TESTS
    logNorm("Testing PMM\n");
    palloc_flags_t* palloc_flags = kmalloc(sizeof(palloc_flags_t));
    palloc_flags->contiguous = 1;
    palloc_flags->page_alloc_num = 128;
    physical_addr_t* paddr_big = kpmalloc(*palloc_flags);
    physical_addr_t* paddr1 = pmalloc();
    printf("PADDR1 %#lx, BIG_ALLOC: %#lx", paddr1, paddr_big);
    if(paddr_big == (physical_addr_t*) 0) kpanic("PMM Test Failed (Big Alloc failed)");

    linear_addr_t* laddr = (linear_addr_t*)0xFFB00000;
    vmm_map_address(laddr, paddr1, 0x3);
    *laddr = 0xB00FBEEF;
    vmm_unmap_address(laddr);

    physical_addr_t* paddr2 = pmalloc();
    vmm_map_address(laddr, paddr2, 0x3);
    printf(", PADDR2 %#lx, AT ADDR: %#lx", paddr2, *laddr);
    pfree(paddr_big, 4096);

    if(*laddr != 0xB00FBEEF) kpanic("PMM Test failed (Mappings Not Equal)");

    physical_addr_t* paddr3 = pmalloc();
    printf("PADDR3 %#lx\n", paddr3);

    if(paddr1 == paddr3) kpanic("PMM Test failed (Equvalence)");
    vmm_unmap_address(laddr);
    kfree(palloc_flags);

    logNorm("Testing VADDM\n");

    linear_addr_t* laddr0 = vmalloc(8);
    linear_addr_t* laddr1 = vmalloc(16);
    linear_addr_t* laddr2 = vmalloc(4096);
    printf("LADDR0: %lx, LADDR1: %lx, LADDR2: %lx\n", laddr0, laddr1, laddr2);
    vfree(laddr1, 16);
    vfree(laddr0, 8);
    laddr1 = vmalloc(16);
    laddr0 = vmalloc(8);
    printf("LADDR0: %lx, LADDR1: %lx, LADDR2: %lx\n", laddr0, laddr1, laddr2);

    if(laddr0 == NULL) kpanic("VADDM Test failed (0-NULL)");
    if(laddr1 == NULL) kpanic("VADDM Test failed (1-NULL)");
    if(laddr2 == NULL) kpanic("VADDM Test failed (2-NULL)");
    if(laddr0 == laddr1) kpanic("VADDM Test failed (0==1)");

    vfree(laddr1, 16);
    vfree(laddr0, 8);
    vfree(laddr2, 4096);
#endif

    process_create(idle_process);
    process_create(yeet);
    process_create(zeet);
    //process_create(kmain);

    //Initialization done, Enable interrupts & timer
    ic_unmaskIRQ(0);
    //asm("xchg %bx, %bx");
    hal_enableInterrupts();

    idle_process();
}

static char* getKernelRelType(udword_t type)
{
    switch(type) {
    case KERNEL_TYPE_ALPHA:   return KERNEL_ALPHA_STR;
    case KERNEL_TYPE_BETA:    return KERNEL_BETA_STR;
    case KERNEL_TYPE_RC:      return KERNEL_RC_STR;
    case KERNEL_TYPE_RELEASE: return KERNEL_RELEASE_STR;
    default:                  return "\0";
    }
}

int yeet()
{
    while(1) {
        terminal_setColor(VGA_BLACK, VGA_GREEN);
        terminal_putchar('a');
        //asm("xchg %bx, %bx");
    }
    return 0;
}

int zeet()
{
    while(1) {
        terminal_setColor(VGA_WHITE, VGA_RED);
        terminal_putchar('B');
    }
    return 0;
}

int kmain()
{
    logNorm("Successfully Initialized kernel\n");
    terminal_puts("\nWelcome to ");
    terminal_puts_Color("Condor!", vga_makeColor(VGA_WHITE, VGA_BLACK));
    printf(" (");
    udword_t* version = getKernelVersion();
    printf(KERNEL_VERSION_FORMATER, version[0], version[1], version[2], getKernelRelType(version[3]));
    printf(")\n");

    while(keyboard_readKey()) asm("pause");

    ubyte_t last_state = 0;

    while(1)
    {
	    ubyte_t new_char = keyboard_readKey();

        if(new_char && keyboard_getKeyState(new_char) != last_state)
        {
            if(keyboard_getChar(new_char)) {
                printf("%c", keyboard_getChar(new_char));
            }
            last_state = keyboard_getKeyState(new_char);
        } else last_state = 0;

        //asm("hlt");
    }

    return 0;
}
