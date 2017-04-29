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
 * File:   vmm.h
 * Author: DropDemBits <r3usrlnd@gmail.com>
 *
 * Created on February 4, 2017, 2:54 PM
 */

#include <condor.h>
#include <kernel/addrs.h>

#ifndef _VMM_H
#define _VMM_H

#define PAGE_PRESENT   0x00000001
#define PAGE_RW        0x00000002
#define PAGE_USER      0x00000004
#define PAGE_FLAG_MASK 0x00000FFF
#define PAGE_ADDR_MASK 0xFFFFF000

/**
 * Maps a linear address to a physical address
 *
 * @param paddr The physical address to map to
 * @param laddr The linear address to map
 * @param flags The flags of the mapping
 * @return 1 if mapping had failed
 */
ubyte_t vmm_map_address(linear_addr_t* laddr, physical_addr_t* paddr, uqword_t flags);

/**
 * Unmaps a linear address
 *
 * @param laddr The linear address to unmap
 * @return The physical address that the linear address mapped to
 */
void vmm_unmap_address(linear_addr_t* laddr);

/**
 * Gets the physical address from a linear address
 * @param laddr The address to get the physical address from
 * @return The physical address mapped to the linear address, or NULL if no mapping was found
 */
physical_addr_t* vmm_get_physical_addr(linear_addr_t* laddr);

/**
 * Initializes the VMM
 */
void vmm_init(void);

/**                                                                                                                                    
 * Switches the PD Pointer to the specified PD                                                                                         
 * @param page_directory_base The PD to point to                                                                                       
 */                                                                                                                                    
void vmm_switchPageBase(udword_t page_directory_base); 

/**
 * Initializes the Virtual Address Manager
 * 
 */
void vaddm_init(void);

/**
 * Sets a region to not be allocated
 * 
 * @param base The base of the region
 * @param size The length of the region
 */
void vaddm_set_region(linear_addr_t base, size_t size);

/**
 * Clears a region to be allocated
 * 
 * @param base The base of the region
 * @param size The length of the region
 */
void vaddm_clear_region(linear_addr_t base, size_t size);

/**
 * Allocates virtual addresses
 * 
 * @param addresses The number of addresses to allocate
 * @return The pointer to the beginning to the allocated addresses
 */
linear_addr_t* valloc(size_t addresses);

/**
 * Returns virtual addresses to be reallocated
 * 
 * @param base_addr The pointer to the beginning to the allocated addresses
 * @param addresses The number of addresses allocated
 */
void vfree(linear_addr_t* base_addr, size_t addresses);


#endif /* _VMM_H */
