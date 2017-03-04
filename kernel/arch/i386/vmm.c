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

#include <kernel/idt.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <condor.h>

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
        PAGE_DIRECTORY[pd_index] = (physical_addr_t)pmalloc() | 0x00000003;
        flush_tlb(laddr);
    }
    if(PAGE_TABLE_BASE[pt_index] & PAGE_PRESENT)
        return 1;
    
    PAGE_TABLE_BASE[pt_index] = ((physical_addr_t)paddr & PAGE_ADDR_MASK) | (flags & PAGE_FLAG_MASK);
    
    flush_tlb(laddr);
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

static void pf_handler(udword_t* esp)
{
    if(!(*(esp+13) & (PAGE_PRESENT | PAGE_USER))) {
        if(pmm_isInited())
            smap_page((linear_addr_t*)readCR2(), pmalloc(), PAGE_PRESENT | PAGE_RW);
        else
        {
            kdump_useStack((uqword_t*)esp);
            printf("ERR: %#lx\n", *(esp+13));
            kpanic("PF Before PMM was initialized");
        }
    }
}

ubyte_t map_address(linear_addr_t* laddr, physical_addr_t* paddr, uqword_t flags)
{
    return smap_page(laddr, paddr, (udword_t)flags);
}

void unmap_address(linear_addr_t* laddr)
{
    pfree(sumap_page(laddr));
    *(&laddr) = 0;
}

physical_addr_t* get_physical(linear_addr_t* laddr)
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

// TODO: Decide whether to section off this region into another file...
/*static udword_t big_bitmap[32];
static udword_t individual_bitmap[32768];

static int vaddm_getFirstClearBit()
{
    int big_index;
    int page_index;
    for()
    {
        
    }
}

linear_addr_t* allocAddrs(size_t length)
{
    
}

void freeAddrs(void* addr, size_t length)
{
    
}

void switchPageBase(udword_t pageAddr)
{
    asm("movl %%0, %eax\n\t" :: "m"(pageAddr));
    switchPageBase();
}*/
