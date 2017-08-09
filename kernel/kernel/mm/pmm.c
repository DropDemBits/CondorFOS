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

/*
 * pmm.c: Physical memory manager
 * Physical management based on bitmaps
 */

#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <condor.h>
#include <multiboot.h>
#include <string.h>
#include <stdio.h>

#define PMM_BASE 0xC8023000

static lomem_bitmaps_t* low_bitmaps = (lomem_bitmaps_t*) PMM_BASE;
static free_stack32_t* stack32 = (free_stack32_t*) (PMM_BASE + 0x1000);
static int32_t stack32_index;
static free_stack64_t* stack64 = (free_stack64_t*) (PMM_BASE + 0x2000);
static int32_t stack64_index;
static ubyte_t is_inited;

/******************************************
*
*  STACK STUFF
*
******************************************/

static void stack_push(physical_addr_t addr)
{
    if(vmm_get_physical_addr((linear_addr_t*) stack32) == 0x0) {
        vmm_map_address((linear_addr_t*) stack32, (physical_addr_t*) addr, PAGE_PRESENT | PAGE_RW);
        memset(stack32, 0xFF, sizeof(free_stack32_t));
        return;
    }

    if(vmm_get_physical_addr((linear_addr_t*) stack64) == 0x0) {
        vmm_map_address((linear_addr_t*) stack64, (physical_addr_t*) addr, PAGE_PRESENT | PAGE_RW);
        memset(stack64, 0xFF, sizeof(free_stack32_t));
        return;
    }

    if(addr == 0xFFFFFFFF) kpanic("Invalid Address Pushed (PMM)");
    else if(addr < 0xFFFFFFFF) {
        if(stack32_index == 1023) {
            uint32_t old_address = (uint32_t) vmm_get_physical_addr((linear_addr_t*) stack32);
            vmm_map_address((linear_addr_t*) stack32, (physical_addr_t*) addr, PAGE_PRESENT | PAGE_RW | PAGE_REMAP);
            memset(stack32, 0xFF, sizeof(free_stack32_t));
            stack32->backlink = old_address;
            stack32_index = 0;
        } else stack32->freeAddrs[stack32_index++] = addr;
    }
    else if(addr > 0xFFFFFFFF) {
        if(stack64_index == 511) {
            uint32_t old_address = (uint32_t) vmm_get_physical_addr((linear_addr_t*) stack64);
            vmm_map_address((linear_addr_t*) stack64, (physical_addr_t*) addr, PAGE_PRESENT | PAGE_RW | PAGE_REMAP);
            memset(stack64, 0xFF, sizeof(free_stack32_t));
            stack64->backlink = old_address;
            stack64_index = 0;
        } else stack64->freeAddrs[stack64_index++] = addr;
    }
}

static physical_addr_t stack_pop64()
{
    physical_addr_t retval;

#if __64_BIT_ADDRESSES__ == 1
    if(stack64_index < 0) {
        retval = (physical_addr_t)vmm_get_physical_addr((linear_addr_t*) stack64);

        if(retval == (physical_addr_t)~0x0) return retval;
        vmm_map_address((linear_addr_t*) stack64, (physical_addr_t*) stack64->backlink, PAGE_PRESENT | PAGE_RW | PAGE_REMAP);
        stack64_index = 1022;
    } else retval = stack64->freeAddrs[--stack64_index];
#else
    //Size is guaranteed to be 32bits
    retval = 0xFFFFFFFF;
#endif

    return retval;
}

static physical_addr_t stack_pop32(palloc_flags_t alloc_flags)
{
    physical_addr_t retval;

    if(stack32_index < 0) {
        retval = (physical_addr_t)vmm_get_physical_addr((linear_addr_t*) stack32);

        if(retval == (physical_addr_t)~0x0) {
            if(alloc_flags.low_4gb) return retval;
            else return stack_pop64();
        }
        vmm_map_address((linear_addr_t*) stack32, (physical_addr_t*) stack32->backlink, PAGE_PRESENT | PAGE_RW | PAGE_REMAP);
        stack32_index = 1022;
    } else retval = stack32->freeAddrs[--stack32_index];

    return retval;
}

/******************************************
*
*  BITMAP STUFF
*
******************************************/

static udword_t check_super_region(udword_t bit)
{
    for(size_t off = 0; off < 16; off++) {
        if(low_bitmaps->block_bitmap[((bit & ~0xF) >> 5) + off] != 0xFFFFFFFF) return 0;
    }
    return 1;
}

static void paddm_set_bit(udword_t bit)
{
    if(bit >= 4096) return;

    low_bitmaps->block_bitmap[bit >> 5] |= (1 << (bit & 0x1F));
	if(check_super_region(bit))
        low_bitmaps->superblock_bitmap[bit >> 9] |= (1 << ((bit >> 4) & 0x1F));
}

static void paddm_clear_bit(udword_t bit)
{
    if(bit >= 4096) return;

    low_bitmaps->block_bitmap[bit >> 5] &= ~(1 << (bit & 0x1F));
	low_bitmaps->superblock_bitmap[bit >> 9] &= ~(1 << ((bit >> 4) & 0x1F));
}

static int paddm_get_bit(udword_t bit)
{
    if(bit >= 4096) return 1;

    return low_bitmaps->block_bitmap[bit >> 5] & (1 << (bit & 0x1F));
}

static int paddm_get_first_clear_bits(palloc_flags_t flags)
{
    if(flags.page_alloc_num == 0 && flags.contiguous) return -1;

    size_t base_bit = 0;
    size_t num_free_bits = 0;

    for(size_t superblock_base = 0; superblock_base < MAX_SUPERBLOCKS; superblock_base++) {
        if(low_bitmaps->superblock_bitmap[superblock_base] != 0xFFFFFFFF) {
            for(size_t super_bit = 0; super_bit < 32; super_bit++) {
                if((low_bitmaps->superblock_bitmap[superblock_base] & (1 << (super_bit & 0x1F))) == 0) {
                    size_t block_base = (superblock_base << 5) | super_bit;
                    if(low_bitmaps->block_bitmap[block_base >> 1] != 0xFFFFFFFF) {
                        for(size_t bit = 0; bit < 16; bit++) {
                            if(paddm_get_bit((block_base << 4) | bit) == 0) {
                                if(num_free_bits++ == 0) base_bit = (block_base << 4) | bit;
                                if(num_free_bits >= flags.page_alloc_num)
                                    return base_bit;
                            }
                            else num_free_bits = 0;
                        }
                    }
                }

                if(flags.align_64kb) {
                    base_bit = 0;
                    num_free_bits = 0;
                }

                if(flags.low_mem && super_bit > 15 && superblock_base == 0) return -1;
            }
        }
    }

    //printf("%lx ", base_bit);
    return -1;
}

/******************************************
*
*  GENERAL STUFF
*
******************************************/

void pmm_init(multiboot_memory_map_t* mmap_base, size_t mmap_length)
{
    if(vmm_get_physical_addr((linear_addr_t*) PMM_BASE) == 0) {
        vmm_map_address((linear_addr_t*) (PMM_BASE + 0x0000), (physical_addr_t*) 0x2000, PAGE_PRESENT | PAGE_RW);
        vmm_map_address((linear_addr_t*) (PMM_BASE + 0x3000), (physical_addr_t*) 0x0000, PAGE_PRESENT);
    }
    kmemset((linear_addr_t*) PMM_BASE, 0xFF, 0x1000);
    kmemset(low_bitmaps->reserved, 0x20, sizeof(low_bitmaps->reserved));

    /**************************************************************************/
    multiboot_memory_map_t* mmap = mmap_base;

    while(((udword_t)mmap) < ((udword_t)mmap_base) + mmap_length) {
        if(mmap->type == MULTIBOOT_MEMORY_ACPI_RECLAIMABLE || mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            pmm_clear_region((udword_t)mmap->addr, (udword_t)mmap->len);
        }
        else if(mmap->type > MULTIBOOT_MEMORY_AVAILABLE) {
            pmm_set_region((udword_t)mmap->addr, (udword_t)mmap->len);
        }

        mmap = (multiboot_memory_map_t*) ((unsigned int)mmap + mmap->size + sizeof(mmap->size));
    }

    /**************************************************************************/
    is_inited = 1;
}

ubyte_t pmm_isInited()
{
    return is_inited;
}

void pmm_clear_region(physical_addr_t base, size_t size)
{
    if(size == 0) return;

    for(udword_t blocks = base >> 12; blocks < (base + size + 0xFFF) >> 12; blocks++) {
        if(blocks >= 0xFFFFF) break;
        else if(blocks >= 0x01000) {
            stack_push(blocks << 12);
        }
        paddm_clear_bit(blocks);
        low_bitmaps->block_bitmap[0] |= 0x00000007;
    }
}

void pmm_set_region(physical_addr_t base, size_t size)
{
    if(size == 0) return;

    for(udword_t blocks = base >> 12; blocks < (base + size + 0xFFF) >> 12; blocks++) {
        if(blocks >= 0x01000) break;
        paddm_set_bit(blocks);
    }
}

physical_addr_t* pmalloc()
{
    static palloc_flags_t alloc_flags;
    return kpmalloc(alloc_flags);
}

physical_addr_t* kpmalloc(palloc_flags_t alloc_flags)
{
    physical_addr_t frame = paddm_get_first_clear_bits(alloc_flags);
    if(frame == 0xFFFFFFFF) return (physical_addr_t*)stack_pop32(alloc_flags);

    paddm_set_bit(frame);

    for(size_t bit = 1; bit < alloc_flags.page_alloc_num && alloc_flags.contiguous; bit++) paddm_set_bit(bit + frame);

    return (physical_addr_t*) (frame << 12);
}

void pfree(physical_addr_t* base_address, size_t num_addresses)
{
    if(base_address == NULL) return;
    else if((physical_addr_t)base_address == 0xFFFFFFFF) kpanic("Invalid address");

    if(base_address >= (physical_addr_t*) 0x01000000 && num_addresses == 1) {
        stack_push((physical_addr_t) base_address);
        return;
    }

    physical_addr_t frame = (physical_addr_t)base_address >> 12;
    if(frame < 4) return;

    for(size_t bit = 0; bit < num_addresses; bit++) paddm_clear_bit(bit + frame);
}
