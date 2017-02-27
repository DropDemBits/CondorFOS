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

static physical_addr_t* stack_base;
static physical_addr_t* stack_limit;
static size_t stack_offset = 0;
static MemoryRegion* regions_base;
static uword_t region_indice = 0;
static ubyte_t is_pmm_init = 0;
static physical_addr_t stackpage_base = 0;
static physical_addr_t stackpage_limit = 0;

static void stack_pushAddr(physical_addr_t addr)
{
    if((addr == 0 && is_pmm_init) || addr == (physical_addr_t) stack_base) return;    
    if(!pmm_isInited() && addr >= stackpage_base && addr <= stackpage_limit) return;
    
    if(stack_offset >= ADDR_PER_BLOCK)
    {
        if(!pmm_isInited())
        {
            stack_base = (physical_addr_t*) stack_base+0x1000;
            stack_offset = 0;
            stack_base[stack_offset++] = (physical_addr_t) stack_base-0x1000;
            stack_base[stack_offset++] = addr;
        } else {
            physical_addr_t prev_stack = (physical_addr_t)stack_base;
            stack_base = (physical_addr_t*) (addr + KERNEL_BASE);
            *(stack_base) = prev_stack;
            stack_offset = 0;
        }
    } else
    {
        stack_base[stack_offset] = (physical_addr_t) addr;
        stack_offset++;
    }
}

static physical_addr_t* stack_popAddr(void)
{
    physical_addr_t* ret_addr = 0;
    
    if(stack_offset == 0)
    {
        ret_addr = (physical_addr_t*)((physical_addr_t)stack_base & 0x3FFFFFFF);
        stack_base = (physical_addr_t*) *(stack_base);
        stack_offset = ADDR_PER_BLOCK-1;
        
        if(*(stack_base) == 0)
            //kpanic("Out of memory");
            return (physical_addr_t*)0;
    }
    else
    {
        ret_addr = (physical_addr_t*) stack_base[--stack_offset];
    }
    
    if(ret_addr == 0)
        //kpanic("Out of memory");
        return (physical_addr_t*)0;
    
    return ret_addr;
}

void pmm_setRegionBase(physical_addr_t region_base)
{
    regions_base = (MemoryRegion*) region_base;
}

void pmm_init(size_t memory_size, physical_addr_t memory_base)
{
    stackpage_base = memory_base;
    stackpage_limit = ((memory_base / ADDR_PER_BLOCK) << 12) + stackpage_base;
    if(memory_base % ADDR_PER_BLOCK)
        stackpage_limit += 0x1000;
        
    stack_base = (physical_addr_t*)memory_base;
    
    stack_limit = stack_base;
    *stack_base = 0;
    
    for(physical_addr_t addr = memory_size - BLOCK_SIZE; addr > 0; addr -= BLOCK_SIZE)
    {
        for(uword_t i = 0; i < region_indice; i++)
        {
            if(addr >= regions_base[i].start_addr && addr <= regions_base[i].end_addr && regions_base[i].flags & 3) {
               addr = regions_base[i].start_addr;
               if(addr <= 0) goto end;
            }
        }
        
        stack_pushAddr(addr);
    }
    end:
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
