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

#include <stdio.h>
#include <string.h>
#include <kernel/idt.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <condor.h>

#include <kernel/stack_state.h>

extern udword_t readCR2();
extern void switchPageBase();
extern udword_t stack_bottom;

static linear_addr_t* PAGE_DIRECTORY  = (linear_addr_t*) 0xFFFFF000;
static linear_addr_t* PAGE_TABLE_BASE = (linear_addr_t*) 0xFFC00000;

static void flush_tlb(linear_addr_t* addr)
{
    asm("invlpg (%0)" :: "r"(addr));
}

static ubyte_t smap_page(linear_addr_t* laddr, physical_addr_t* paddr, uword_t flags)
{
    udword_t pd_index = (linear_addr_t)laddr >> (BLOCK_BITS+10);
    udword_t pt_index = (linear_addr_t)laddr >> BLOCK_BITS;
    
    if(!(PAGE_DIRECTORY[pd_index] & PAGE_PRESENT))
    {
        PAGE_DIRECTORY[pd_index] = (physical_addr_t)pmalloc() | 0x00000003 | (flags & PAGE_USER);
        
        flush_tlb(laddr);
        memset((linear_addr_t*)((linear_addr_t)(PAGE_TABLE_BASE+pt_index) & (~PAGE_FLAG_MASK)), 0, 4096);
        flush_tlb(laddr);
    }
    if(PAGE_TABLE_BASE[pt_index] & PAGE_PRESENT)
        return 1;
    
    PAGE_TABLE_BASE[pt_index] = ((physical_addr_t)paddr & PAGE_ADDR_MASK) | (flags & PAGE_FLAG_MASK);
    
    flush_tlb(laddr);
    memset(laddr, 0, 4096);
    return 0;
}

static physical_addr_t* sumap_page(linear_addr_t* laddr)
{
    udword_t pd_index = (linear_addr_t)laddr >> (BLOCK_BITS+10);
    udword_t pt_index = (linear_addr_t)laddr >> BLOCK_BITS;
    
    if(!(PAGE_DIRECTORY[pd_index] & PAGE_PRESENT))
        return 0;
    if(!(PAGE_TABLE_BASE[pt_index] & PAGE_PRESENT))
        return 0;
    
    physical_addr_t* retAddr = (physical_addr_t*)(PAGE_TABLE_BASE[pt_index] & PAGE_ADDR_MASK);
    PAGE_TABLE_BASE[pt_index] = 0;
    flush_tlb(laddr);
    
    return retAddr;
}

static void pf_handler(stack_state_t* state)
{
    if(!(state->err_code & (PAGE_PRESENT | PAGE_USER))) {
        if(pmm_isInited()) {
            smap_page((linear_addr_t*)readCR2(), pmalloc(), PAGE_PRESENT | PAGE_RW);
        }
        else {
            kdump_useStack(state);
            printf("ERR: %#lx\n", state->err_code);
            kpanic("PF Before PMM was initialized");
        }
    }
}

ubyte_t vmm_map_address(linear_addr_t* laddr, physical_addr_t* paddr, uqword_t flags)
{
    return smap_page(laddr, paddr, (udword_t)flags);
}

void vmm_unmap_address(linear_addr_t* laddr)
{
    pfree(sumap_page(laddr));
    *(&laddr) = 0;
}

physical_addr_t* vmm_get_physical_addr(linear_addr_t* laddr)
{
    udword_t pd_index = (linear_addr_t)laddr >> (BLOCK_BITS+10);
    udword_t pt_index = (linear_addr_t)laddr >> BLOCK_BITS;
    
    if(!(PAGE_DIRECTORY[pd_index] & PAGE_PRESENT))
        return 0;
    if(!(PAGE_TABLE_BASE[pt_index] & PAGE_PRESENT))
        return 0;
    
    return (physical_addr_t*) ((PAGE_TABLE_BASE[pt_index] & ~0xFFF) | ((linear_addr_t)laddr & 0xFFF));
}

void vmm_init()
{
    idt_addISR(14, (udword_t)pf_handler);
}

void vmm_switchPageBase(udword_t page_directory_base)
{
    asm("movl %0, %%eax\n\t" :: "m"(page_directory_base));
    switchPageBase();
}

// TODO: Decide whether to section off this region into another file...
static udword_t* superpage_bitmap;
static udword_t* page_bitmap;

static void vaddm_set_bit(udword_t bit) {
    if(bit > 32768) return;
    page_bitmap[bit >> 5] |= (1 << (bit % 32));
	if(page_bitmap[bit >> 6] == 0xFFFFFFFF && page_bitmap[(bit >> 6) + 1] == 0xFFFFFFFF)
        superpage_bitmap[bit >> 15] |= (1 << ((bit >> 10) % 32));
}

static void vaddm_clear_bit(udword_t bit) {
    if(bit > 32768) return;
    page_bitmap[bit >> 5] &= ~(1 << (bit % 32));
	superpage_bitmap[bit >> 15] &= ~(1 << ((bit >> 10) % 32));
}

static int vaddm_get_bit(udword_t bit) {
    if(bit > 32768) return 1;
    return page_bitmap[bit >> 5] >> (bit % 32);
}

static int vaddm_get_first_clear_bits(size_t num_bits)
{
    for(size_t superpage_base = 0; superpage_base < 32; superpage_base++) {
        if(superpage_bitmap[superpage_base] != 0xFFFFFFFF) {
            for(size_t page_base = superpage_base << 6; page_base < 32768; page_base++) {
                if(page_bitmap[page_base] != 0xFFFFFFFF) {
                    size_t num_free_bits = 0;
                    for(size_t bit = page_base << 5; bit < (page_base << 5) + 32; bit++) {
                        if(num_free_bits >= num_bits) break;
                        else if(vaddm_get_bit(bit) != 1) {
                            num_free_bits++;
                        }
                        else num_free_bits = 0;
                    }
                    
                    if(num_free_bits >= num_bits) {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

linear_addr_t* vmalloc(size_t num_addresses)
{
	if(num_addresses == 0) return (linear_addr_t*) 0;
	
    linear_addr_t frame = vaddm_get_first_clear_bits(num_addresses);
    if(frame == 0) return (linear_addr_t*) 0;
    
    for(size_t offset = 0; offset < num_addresses; offset++) {
    	vaddm_set_bit(frame + offset);
    }
    
    return (linear_addr_t*) (frame << 12);
}

void vfree(linear_addr_t* base_address, size_t num_addresses)
{
    if(num_addresses == 0 || base_address == NULL) return;
	
    linear_addr_t frame = (linear_addr_t)base_address >> 12;
    
    for(size_t offset = 0; offset < offset; offset++) {
    	vaddm_clear_bit(frame + offset);
    }
}

void vaddm_clear_region(linear_addr_t base, size_t size)
{
    if(base == 0 || size == 0) return;
    memset((linear_addr_t*) (page_bitmap + (base >> 17)), 0x0, (base-size + 0xFFF) >> 12);
    memset((linear_addr_t*) (superpage_bitmap + (base >> 27)), 0x0, (base-size + 0xFFF) >> 22);
    
    page_bitmap[0] |= 0x1;
}

void vaddm_set_region(linear_addr_t base, size_t size)
{
    if(base == 0 || size == 0) return;
    memset((linear_addr_t*) (page_bitmap + (base >> 17)), 0xF, (base-size + 0xFFF) >> 12);
    memset((linear_addr_t*) (superpage_bitmap + (base >> 27)), 0xF, (base-size + 0xFFF) >> 22);
    
    page_bitmap[0] |= 0x1;
}

void vaddm_init(linear_addr_t vaddm_base)
{
    if(!pmm_isInited()) return;
    
    if(vmm_get_physical_addr((linear_addr_t*) vaddm_base) == 0) {
        vmm_map_address((linear_addr_t*) (vaddm_base + 0x0000), pmalloc(), PAGE_RW | PAGE_PRESENT);
        vmm_map_address((linear_addr_t*) (vaddm_base + 0x1000), pmalloc(), PAGE_RW | PAGE_PRESENT);
        vmm_map_address((linear_addr_t*) (vaddm_base + 0x2000), pmalloc(), PAGE_RW | PAGE_PRESENT);
        vmm_map_address((linear_addr_t*) (vaddm_base + 0x3000), pmalloc(), PAGE_RW | PAGE_PRESENT);
        vmm_map_address((linear_addr_t*) (vaddm_base + 0x4000), pmalloc(), PAGE_RW | PAGE_PRESENT);
    }
    
    page_bitmap = (udword_t*) vaddm_base;
    superpage_bitmap = (udword_t*) vaddm_base + 0x4000;
    
    vaddm_set_region(KERNEL_VIRTUAL_BASE, 0xFFFFFFFF - KERNEL_VIRTUAL_BASE);
    vaddm_set_region(0x0, 0x100000);
    
    page_bitmap[0] |= 0x1;
}
