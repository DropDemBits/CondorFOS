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
extern udword_t readCR3();

udword_t current_CR3 = 0x0;

static void map_page(linear_addr_t laddr, physical_addr_t paddr, uword_t flags)
{
    //TODO: Finalize page mapping
    uword_t pd_index = laddr >> 22;
    udword_t* pd = (udword_t*) (current_CR3 & 0xFFFFF000);
    if(pd[pd_index] & 0xFFFFFFFE)
        kpanic("PDE Nonexistant\n");
    
    uword_t pt_index = (laddr >> 12) & 0x3FF;
    udword_t* pt = (udword_t*) pd[pd_index];
    if(pt[pt_index] & 0x1)
        kpanic("PTE Exists\n");
    
    pt[pt_index] = (paddr & 0xFFFFF000) | (flags & 0xFFF);
    
}

static void pf_handler(udword_t* esp)
{
    printf("\nERR: %lx\n", *(esp+13));
    if(*(esp+13) == 0x000) {
        if(pmm_isInited())
            map_page(readCR2(), (physical_addr_t)pmalloc(), 0x3);
        else
            map_page(readCR2(), (physical_addr_t)readCR2()-KERNEL_VIRTUAL_BASE, 0x3);
    }
}

void vmm_init()
{
    current_CR3 = readCR3();
    if(!current_CR3) kpanic("CR3 is zero (paging not setup?)");
    
    idt_addISR(14, (udword_t)pf_handler);
}
