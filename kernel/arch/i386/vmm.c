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

#define VADDM_BASE 0xC8000000

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
        if(pmm_isInited()) 
            PAGE_DIRECTORY[pd_index] = (physical_addr_t)pmalloc(1) | 0x00000003 | (flags & PAGE_USER);
        else
            PAGE_DIRECTORY[pd_index] = 0x6000 | 0x00000003 | (flags & PAGE_USER);
        
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
    linear_addr_t* laddr = (linear_addr_t*)readCR2();
    
    if(!(state->err_code & (PAGE_PRESENT | PAGE_USER))) {
        if(!pmm_isInited()) {
            kdump_useStack(state);
            printf("ERR: %#lx\n", state->err_code);
            kpanic("PF Before PMM was initialized");
        }
        else if(vmm_get_physical_addr(laddr) != 0) {
            PAGE_TABLE_BASE[(linear_addr_t)laddr >> BLOCK_BITS] |= PAGE_PRESENT;
        }
        else smap_page(laddr, pmalloc(1), PAGE_PRESENT | PAGE_RW);
    }
}

ubyte_t vmm_map_address(linear_addr_t* laddr, physical_addr_t* paddr, uqword_t flags)
{
    if(paddr == 0) return 1;
    return smap_page(laddr, paddr, (udword_t)flags);
}

void vmm_unmap_address(linear_addr_t* laddr)
{
    pfree(sumap_page(laddr), 1);
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
    if(bit > 32768*32) return;
    
    page_bitmap[bit / 32] |= (1 << (bit % 32));
	if(page_bitmap[bit >> 6] == 0xFFFFFFFF && page_bitmap[(bit >> 6) + 1] == 0xFFFFFFFF)
        superpage_bitmap[bit >> 15] |= (1 << ((bit >> 10) % 32));
}

static void vaddm_clear_bit(udword_t bit) {
    if(bit > 32768*32) return;
    
    page_bitmap[bit >> 5] &= ~(1 << (bit % 32));
	superpage_bitmap[bit >> 15] &= ~(1 << ((bit >> 10) % 32));
}

int vaddm_get_bit(udword_t bit) {
    if(bit > 32768*32) return 1;
    
    return page_bitmap[bit >> 5] & (1 << (bit % 32));
}

static int vaddm_get_first_clear_bits(size_t num_bits)
{
    if(num_bits == 0) return -1;
    
    size_t base_bit = 0;
    size_t num_free_bits = 0;
    
    for(size_t superpage_base = 0; superpage_base < 32; superpage_base++) {
        if(superpage_bitmap[superpage_base] != 0xFFFFFFFF) {
            for(size_t page_base = superpage_base << 6; page_base < 32768; page_base++) {
                if(page_bitmap[page_base] != 0xFFFFFFFF) {
                    for(size_t bit = 0; bit < 32; bit++) {
                        if(vaddm_get_bit(bit + page_base) == 0) {
                            if(num_free_bits++ == 0) base_bit = (page_base) + bit;
                            
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

linear_addr_t* valloc(size_t num_addresses)
{
    if(num_addresses == 0) return (linear_addr_t*) 0;
	
    linear_addr_t frame = vaddm_get_first_clear_bits(num_addresses);
    if(frame == 0xFFFFFFFF) return (linear_addr_t*) 0;
    
    for(size_t bit = frame; bit < num_addresses + frame; bit++) vaddm_set_bit(bit);
    
    return (linear_addr_t*) (frame << 12);
}

void vfree(linear_addr_t* base_address, size_t num_addresses)
{
    if(num_addresses == 0 || base_address == NULL) return;
	
    linear_addr_t frame = (linear_addr_t)base_address >> 12;
    
    for(size_t bit = frame; bit < num_addresses + frame; bit++) vaddm_clear_bit(bit);
}

void vaddm_clear_region(linear_addr_t base, size_t size)
{
    if(size == 0) return;
    for(udword_t blocks = base >> 12; blocks < (size + 0xFFF) >> 12; blocks++) vaddm_clear_bit(blocks);
    
    page_bitmap[0] |= 0x1;
}

void vaddm_set_region(linear_addr_t base, size_t size)
{
    if(size == 0) return;
    for(udword_t blocks = base >> 12; blocks < (size + 0xFFF) >> 12; blocks++) vaddm_set_bit(blocks);
    
    page_bitmap[0] |= 0x1;
}

void vaddm_init()
{
    if(!pmm_isInited()) return;
    
    if(vmm_get_physical_addr((linear_addr_t*) VADDM_BASE) == 0) {
        ubyte_t retval = 0;
        
        retval |= vmm_map_address((linear_addr_t*) (VADDM_BASE + 0x0000), pmalloc(1), PAGE_RW | PAGE_PRESENT);
        retval |= vmm_map_address((linear_addr_t*) (VADDM_BASE + 0x1000), pmalloc(1), PAGE_RW | PAGE_PRESENT);
        retval |= vmm_map_address((linear_addr_t*) (VADDM_BASE + 0x2000), pmalloc(1), PAGE_RW | PAGE_PRESENT);
        retval |= vmm_map_address((linear_addr_t*) (VADDM_BASE + 0x3000), pmalloc(1), PAGE_RW | PAGE_PRESENT);
        retval |= vmm_map_address((linear_addr_t*) (VADDM_BASE + 0x4000), pmalloc(1), PAGE_RW | PAGE_PRESENT);
        
        if(retval) kpanic("VADDM Page Mapping failure");
    }
    
    page_bitmap = (udword_t*) VADDM_BASE;
    superpage_bitmap = (udword_t*) VADDM_BASE + 0x4000;
    
    vaddm_clear_region(0x0, 0xFFFFFFFF);
    vaddm_set_region(KERNEL_VIRTUAL_BASE, 0xFFFFFFFF - KERNEL_VIRTUAL_BASE);
    vaddm_set_region(0x0, 0x100000);
    
    page_bitmap[0] |= 0x1;
}
