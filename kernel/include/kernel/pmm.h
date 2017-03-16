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

#ifndef _PMM_H
#define _PMM_H

/**
 * Memory region structure
 */
typedef struct {
    physical_addr_t start_addr;
    physical_addr_t end_addr;
    uchar_t flags;
} MemoryRegion;

/**
 * The size of an allocated block
 */
#define BLOCK_SIZE 0x1000

/**
 * Bit shift offset to the beginning of the actual address
 */
#define BLOCK_BITS 0xC

#define ADDR_PER_BLOCK (BLOCK_SIZE / sizeof(physical_addr_t))-1

/**
 * void pmm_setRegionBase(physical_addr_t region_base)
 *
 * Sets the region base
 * @param region_base The base address of the region structures
 */
void pmm_setRegionBase(physical_addr_t region_base);

/**
 * void pmm_init(size_t memory_size, size_t bitmap_location);
 *
 * Initializes the PMM
 * @param memory_size The size of the physical memory
 * @param bitmap_location The destination of the bitmap
 */
void pmm_init(size_t memory_size, physical_addr_t bitmap_location);

/**
 * void pmm_setRegion(physical_addr_t region_start, size_t region_size);
 *
 * Sets a region in the bitmap for preventing allocation
 * @param region_start The physical address of the region to set
 * @param region_size The size of the region to set
 */
void pmm_setRegion(physical_addr_t region_start, size_t region_size);

/**
 * void pmm_clearRegion(physical_addr_t region_start, size_t region_size);
 *
 * Clears a region in the bitmap for allocation
 * @param region_start The physical address of the region to clear
 * @param region_size The size of the region to clear
 */
void pmm_clearRegion(physical_addr_t region_start, size_t region_size);

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
physical_addr_t* pmalloc(void);

/**
 * void pfree(physical_ptr_t address);
 *
 * Frees an address to be reused
 * @param address The address to free
 */
void pfree(physical_addr_t* address);

#endif /** PMM_ H */
