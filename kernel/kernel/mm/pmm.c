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
#include <string.h>
#include <stdio.h>

#define PMM_BASE 0xC8005000

static udword_t* superblock_bitmap;
static udword_t* block_bitmap;
static ubyte_t is_inited;

static void paddm_set_bit(udword_t bit) {
    if(bit > 32768*32) return;
    
    block_bitmap[bit / 32] |= (1 << (bit % 32));
	if(block_bitmap[(bit >> 5) & ~0x1] == 0xFFFFFFFF && block_bitmap[(bit >> 5) | 0x1] == 0xFFFFFFFF)
        superblock_bitmap[bit >> 15] |= (1 << ((bit >> 10) % 32));
}

static void paddm_clear_bit(udword_t bit) {
    if(bit > 32768*32) return;
    
    block_bitmap[bit >> 5] &= ~(1 << (bit % 32));
	superblock_bitmap[bit >> 15] &= ~(1 << ((bit >> 10) % 32));
}

int paddm_get_bit(udword_t bit) {
    if(bit > 32768*32) return 1;
    
    return block_bitmap[bit >> 5] & (1 << (bit % 32));
}

static int paddm_get_first_clear_bits(size_t num_bits)
{
    if(num_bits == 0) return -1;
    
    size_t base_bit = 0;
    size_t num_free_bits = 0;
    
    for(size_t superblock_base = 0; superblock_base < 32; superblock_base++) {
        if(superblock_bitmap[superblock_base] != 0xFFFFFFFF) {
            for(size_t block_base = superblock_base << 6; block_base < 32768; block_base++) {
                if(block_bitmap[block_base] != 0xFFFFFFFF) {
                    for(size_t bit = 0; bit < 32; bit++) {
                        
                        if(paddm_get_bit(bit + block_base) == 0) {
                            if(num_free_bits++ == 0) base_bit = (block_base) + bit;
                            
                            if(num_free_bits >= num_bits) return base_bit;
                        }
                        else num_free_bits = 0;
                    }
                }
            }
        }
    }
    
    return -1;
}

void pmm_init()
{
    if(vmm_get_physical_addr((linear_addr_t*) PMM_BASE) == 0) {
        vmm_map_address((linear_addr_t*) (PMM_BASE + 0x0000), (physical_addr_t*) 0x1000, PAGE_PRESENT | PAGE_RW);
        vmm_map_address((linear_addr_t*) (PMM_BASE + 0x1000), (physical_addr_t*) 0x2000, PAGE_PRESENT | PAGE_RW);
        vmm_map_address((linear_addr_t*) (PMM_BASE + 0x2000), (physical_addr_t*) 0x3000, PAGE_PRESENT | PAGE_RW);
        vmm_map_address((linear_addr_t*) (PMM_BASE + 0x3000), (physical_addr_t*) 0x4000, PAGE_PRESENT | PAGE_RW);
        vmm_map_address((linear_addr_t*) (PMM_BASE + 0x4000), (physical_addr_t*) 0x5000, PAGE_PRESENT | PAGE_RW);
    }
    
    block_bitmap = (udword_t*) PMM_BASE;
    superblock_bitmap = (udword_t*) (PMM_BASE + 0x4000);
    
    kmemset((linear_addr_t*) PMM_BASE, 0xFF, 0x5000);
    
    is_inited = 1;
}

ubyte_t pmm_isInited()
{
    return is_inited;
}

void pmm_clear_region(physical_addr_t base, size_t size)
{
    if(size == 0) return;
    
    for(udword_t blocks = base >> 12; blocks < (size + 0xFFF) >> 12; blocks++) paddm_clear_bit(blocks);
    
    block_bitmap[0] |= 0x7F;
}

void pmm_set_region(physical_addr_t base, size_t size)
{
    if(size == 0) return;
    
    for(udword_t blocks = base >> 12; blocks < (size + 0xFFF) >> 12; blocks++) paddm_set_bit(blocks);
    
    block_bitmap[0] |= 0x7F;
}

physical_addr_t* pmalloc(size_t num_addresses)
{
	if(num_addresses == 0) return NULL;
    
    physical_addr_t frame = paddm_get_first_clear_bits(num_addresses);
    if(frame == 0xFFFFFFFF || frame < 7) return NULL;
    
    for(size_t bit = 0; bit < num_addresses; bit++) paddm_set_bit(bit + frame);
    
    return (physical_addr_t*) (frame << 12);
}

void pfree(physical_addr_t* base_address, size_t num_addresses)
{
    if(num_addresses == 0 || base_address == NULL) return;
	
    physical_addr_t frame = (physical_addr_t)base_address >> 12;
    
    for(size_t bit = 0; bit < num_addresses; bit++) paddm_clear_bit(bit + frame);
}
