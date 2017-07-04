/*
 * Copyright (C) 2017 DropDemBits
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
 * File:   pmm.h
 * Author: DrompDemBits
 *
 * Created on January 5, 2017, 5:56 PM
 */

#include <kernel/addrs.h>
#include <condor.h>
#include <multiboot.h>

#ifndef _PMM_H
#define _PMM_H

/**
 * The size of an allocated block
 */
#define BLOCK_SIZE 0x1000

#define BLOCK_BITS 12

#define MAX_BLOCKS 128

#define MAX_SUPERBLOCKS 8

#define PADDING_SIZE (0x400 - (MAX_BLOCKS + MAX_SUPERBLOCKS))

typedef struct {
    uint32_t block_bitmap[MAX_BLOCKS];
    uint32_t superblock_bitmap[MAX_SUPERBLOCKS];
    uint32_t reserved[PADDING_SIZE];
} lomem_bitmaps_t;

typedef struct {
    uint32_t backlink;
    uint32_t freeAddrs[1023];
} free_stack32_t;

typedef struct {
    uint64_t backlink;
    uint64_t freeAddrs[511];
} free_stack64_t;

typedef struct {
    uint64_t align_64kb : 1;
    uint64_t low_mem : 1;
    uint64_t contiguous : 1;
    uint64_t big_page : 1;
    uint64_t low_4gb : 1;
    uint64_t reserved : 11;
    uint64_t page_alloc_num : 16;
} palloc_flags_t;

/**
 * void pmm_init(void);
 *
 * Initializes the PMM
 * @param mmap_base The pointer to the base of the memory map
 */
void pmm_init(multiboot_memory_map_t* mmap_base, size_t mmap_length);

/**
 * void pmm_setRegion(physical_addr_t region_start, size_t region_size);
 *
 * Sets a region in the bitmap for preventing allocation
 * @param region_start The physical address of the region to set
 * @param region_size The size of the region to set
 */
void pmm_set_region(physical_addr_t region_start, size_t region_size);

/**
 * void pmm_clear_region(physical_addr_t region_start, size_t region_size);
 *
 * Clears a region in the bitmap for allocation
 * @param region_start The physical address of the region to clear
 * @param region_size The size of the region to clear
 */
void pmm_clear_region(physical_addr_t region_start, size_t region_size);

/**
 * pmm_isInited(void);
 *
 * Checks if the PMM has initialized
 * @return The initialization status of the PMM
 */
ubyte_t pmm_isInited(void);

/**
 * physical_addr_t* pmalloc();
 *
 * Allocates page size blocks
 * @return The address to the allocated block
 */
physical_addr_t* pmalloc();

/**
 * physical_addr_t* kpmalloc(palloc_flags_t alloc_flags);
 *
 * Allocates page size blocks
 * @param alloc_flags The conditions of allocting a block
 * @return The address to the allocated block(s)
 */
physical_addr_t* kpmalloc(palloc_flags_t alloc_flags);

/**
 * void pfree(physical_addr_t* address, size_t num_addresses);
 *
 * Frees an address to be reused
 * @param address The address to free
 * @param num_addresses The number of addresses that have been allocated
 */
void pfree(physical_addr_t* address, size_t num_addresses);

#endif /** PMM_ H */
