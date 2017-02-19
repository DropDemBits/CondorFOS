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
 * Physical management based on stacks
 */

#include <kernel/pmm.h>
#include <condor.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    physical_addr_t start_addr;
    physical_addr_t end_addr;
    uchar_t flags;
} MemoryRegion;

static physical_addr_t* stack_base;
static uword_t stack_offset = 0;
static MemoryRegion* regions_base;
static uword_t region_indice = 0;
static ubyte_t is_pmm_init = 0;

static void stack_pushAddr(physical_addr_t addr)
{
    if((addr == 0 && is_pmm_init) || addr == (physical_addr_t) stack_base) return;
    
    stack_base[stack_offset] = (physical_addr_t) addr;
    stack_offset++;
}

static physical_addr_t* stack_popAddr()
{
    physical_addr_t* ret_addr = 0;
    
    stack_offset--;
    ret_addr = (physical_addr_t*) stack_base[stack_offset];
    
    if(ret_addr == 0)
        kpanic("Out of memory");
    
    return ret_addr;
}

void pmm_setRegionBase(physical_addr_t region_base)
{
    regions_base = (MemoryRegion*) region_base;
}

void pmm_init(size_t memory_size, physical_addr_t memory_base)
{
    printf("0x%lx\n", memory_size);
    stack_base = (physical_addr_t*)memory_base;
    
    asm("xchg %bx, %bx");
    stack_pushAddr(0);
    
    for(physical_addr_t addr = 0; addr <= memory_size; addr += 4096)
    {
        for(uword_t i = 0; i < region_indice; i++)
        {
            if(addr >= regions_base[i].start_addr && addr <= regions_base[i].end_addr && regions_base[i].flags & 3) {
               addr = regions_base[i].end_addr;
            }
        }
        
        stack_pushAddr(addr);
    }
    is_pmm_init = 1;
}

void pmm_setRegion(physical_addr_t region_start, size_t region_size)
{
    regions_base[region_indice].start_addr = region_start;
    if((region_start+region_size) & 0xFFF)
        regions_base[region_indice].end_addr = ((region_start+region_size) & 0xFFFFF000) + 0x1000;
    else
        regions_base[region_indice].end_addr = ((region_start+region_size) & 0xFFFFF000);
    
    regions_base[region_indice].flags = 0x00000003;
    region_indice++;
}

void pmm_clearRegion(physical_addr_t region_start, size_t region_size)
{
    regions_base[region_indice].start_addr = region_start;
    if((region_start+region_size) & 0xFFF)
        regions_base[region_indice].end_addr = ((region_start+region_size) & 0xFFFFF000) + 0x1000;
    else
        regions_base[region_indice].end_addr = ((region_start+region_size) & 0xFFFFF000);
    
    regions_base[region_indice].flags = 0x00000001;
    region_indice++;
}

ubyte_t pmm_isInited()
{
    return is_pmm_init & 1;
}

physical_addr_t* pmalloc(void)
{
    return stack_popAddr();
}

void pfree(physical_addr_t* address)
{
    stack_pushAddr((physical_addr_t)address);
}
