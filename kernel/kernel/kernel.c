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
		logFErr("Not loaded by a multiboot bootloader");
		idle_process();
    }

    printf("Kernel (Start: %lx, End: %lx)\n", &kernel_start, &kernel_end);

    if(info_struct->flags & MULTIBOOT_INFO_MEMORY) {
        printf("Memory Size: Low: %ldkb, %ldkb, Total: %ldMiB\n", info_struct->mem_lower, info_struct->mem_upper, (info_struct->mem_lower+info_struct->mem_upper)/1024);
    }

    logNorm("Initializing VMM\n");
    vmm_init();

    if(info_struct->flags & MULTIBOOT_INFO_MEM_MAP) {
        multiboot_memory_map_t* mmap = (multiboot_memory_map_t*) (info_struct->mmap_addr | KERNEL_BASE);
        multiboot_module_t* module = (multiboot_module_t*) (info_struct->mods_addr | KERNEL_BASE);

        logNorm("Initializing PMM\n");
        pmm_init(mmap, info_struct->mmap_length);

        //Remove regions already occupied/mapped
        pmm_set_region((linear_addr_t)&kernel_start - KERNEL_BASE, (size_t)&kernel_end - KERNEL_BASE);
        //For now, reserve stuff from multiboot info
        pmm_set_region(((linear_addr_t) info_struct) & ~KERNEL_BASE, sizeof(multiboot_info_t));

        //Reserve spaces occupied by modules, and stuff pointed to
        for(size_t i = 0; i < info_struct->mods_count; i++, module++) {
            pmm_set_region((physical_addr_t) vmm_get_physical_addr((linear_addr_t*)module), sizeof(multiboot_module_t));
            pmm_set_region(module->mod_start, module->mod_start - module->mod_end);
            pmm_set_region(module->cmdline, 1);
        }

        while(((udword_t)mmap) < (info_struct->mmap_addr | KERNEL_BASE) + info_struct->mmap_length) {
            //Print MMAP
            printf("base_addr = 0x%lx, length = 0x%lx, %s\n", (udword_t)mmap->addr, (udword_t)mmap->len, mmap_types[mmap->type]);
            mmap = (multiboot_memory_map_t*) ((unsigned int)mmap + mmap->size + sizeof(mmap->size));
        }
    }
    logNorm("Initializing Core VADDM\n");
    vaddm_init();

    logNorm("Initializing Timer\n");
    hal_initTimer();

    /*
     * These two don't have to be initialized right now. They can wait after
     * USB initialization, but we haven't initialized USB devices yet, so leave
     * as be.
     */
    logNorm("Initializing Device Controller\n");
    hal_initController();

    logNorm("Initializing keyboard\n");
    keyboard_init();

#if _ARCH_IDE_HAS_DEFAULTS_ == 1
    logNorm("Initializing IDE Controllers\n");
    uword_t ide_dev0 = ATA_DEVICE_INVALID, ide_dev1 = ATA_DEVICE_INVALID;

    ide_dev0 = ide_init(0x1F0, 0x3F6, 14);
    ide_dev1 = ide_init(0x170, 0x376, 15);

    if(ide_dev0 != ATA_DEVICE_INVALID) printf("IDE Device on Primary Bus (dev_id: %d)\n", ide_dev0);
    if(ide_dev1 != ATA_DEVICE_INVALID) printf("IDE Device on Secondary Bus (dev_id: %d)\n", ide_dev1);

    linear_addr_t* data = kmalloc(4096);
    memset(data, 0x00, 4096);
    if(ata_readSectors(ide_dev0, 0, 1, data)) printf("(Bus0,ATA) This is data: %lx%lx%lx%lx\n", data[0], data[1], data[2], data[3]);
    memset(data, 0x00, 4096);
    if(ata_readSectors(ide_dev1, 0, 1, data)) printf("(Bus1,ATA) This is data: %lx%lx%lx%lx\n", data[0], data[1], data[2], data[3]);
    memset(data, 0x00, 4096);
    if(atapi_readSectors(ide_dev0, 0, 1, data)) printf("(Bus0,ATAPI) This is data: %lx%lx%lx%lx\n", data[0], data[1], data[2], data[3]);
    memset(data, 0x00, 4096);
    if(atapi_readSectors(ide_dev1, 0, 1, data)) printf("(Bus1,ATAPI) This is data: %lx%lx%lx%lx\n", data[0], data[1], data[2], data[3]);
    kfree(data);
#endif

    if(info_struct->flags & MULTIBOOT_INFO_BOOT_LOADER_NAME) {
        logInfo("Loaded by Multiboot loader ");
        printf("%s\n", info_struct->boot_loader_name + KERNEL_BASE);
        serial_writes(COM1, (char*)info_struct->boot_loader_name + KERNEL_BASE);
        serial_writechar(COM1, '\n');
        serial_writechar(COM1, '\r');
    }

    linear_addr_t* test_laddr = (linear_addr_t*)0xFFFFFFFF;

    if(info_struct->flags & MULTIBOOT_INFO_MODS && info_struct->mods_count) {
        logInfo("Bootloader has loaded modules\n");
        printf("Number of modules: %d\n", info_struct->mods_count);

        multiboot_module_t* module = (multiboot_module_t*) (info_struct->mods_addr | KERNEL_BASE);
        linear_addr_t* cmdline = (linear_addr_t*) 0x04000000;

        for(size_t i = 0; i < info_struct->mods_count; i++, module++) {
            vmm_map_address(cmdline, (physical_addr_t*) module->cmdline, PAGE_PRESENT | PAGE_USER | PAGE_REMAP);
            cmdline = (linear_addr_t*)((linear_addr_t)cmdline | (module->cmdline & 0xFFF));
            if(strncmp((char*) cmdline, "/test.bin", strlen("/test.bin")) == 0) {
                test_laddr = vmalloc(1);
                vmm_map_address(test_laddr, (physical_addr_t*)module->mod_start, PAGE_PRESENT | PAGE_USER);
            }
            else if(strncmp((char*) cmdline, "/initrd.tar.gz", strlen("/initrd.tar.gz")) == 0)
                printf("This is initrd.tar.gz\n");
            else
                printf("Unknown module %s\n", cmdline);
        }

        vmm_map_address(cmdline, (physical_addr_t*) 0, PAGE_REMAP);
    }

    process_create(idle_process, false);
    if(test_laddr != (linear_addr_t*)0xFFFFFFFF) {
        process_create(test_laddr, true);
    }
    process_create(kmain, false);

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
    printf(", PADDR2 %#lx, AT ADDR: %#lx\n", paddr2, *laddr);
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

    if(laddr0 == (linear_addr_t*)0xFFFFFFFF) kpanic("VADDM Test failed (0-NULL)");
    if(laddr1 == (linear_addr_t*)0xFFFFFFFF) kpanic("VADDM Test failed (1-NULL)");
    if(laddr2 == (linear_addr_t*)0xFFFFFFFF) kpanic("VADDM Test failed (2-NULL)");
    if(laddr0 == laddr1) kpanic("VADDM Test failed (0==1)");

    vfree(laddr1, 16);
    vfree(laddr0, 8);
    vfree(laddr2, 4096);
#endif

    //Initialization done, Enable interrupts & timer
    ic_unmaskIRQ(0);
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

int kmain()
{
    logNorm("Successfully Initialized kernel\n");
    terminal_puts("\nWelcome to ");
    terminal_puts_Color("Condor!", vga_makeColor(VGA_WHITE, VGA_BLACK));
    printf(" (");
    udword_t* version = getKernelVersion();
    printf(KERNEL_VERSION_FORMATER, version[0], version[1], version[2], getKernelRelType(version[3]));
    printf(")\n");
    ubyte_t last_state = 0;

    keyboard_resetState();

    while(1) {
        ubyte_t new_char = keyboard_readKey();

        if(new_char && keyboard_getKeyState(new_char) != last_state) {
            if(keyboard_getChar(new_char)) {
                printf("%c", keyboard_getChar(new_char));
            }
            last_state = keyboard_getKeyState(new_char);
        } else last_state = 0;
    }

    return 0;
}
