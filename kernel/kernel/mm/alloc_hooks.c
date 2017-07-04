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

/**
 * alloc_hooks.c: Hooks for the liballoc allocator
 */
 #include <kernel/vmm.h>
 #include <kernel/pmm.h>

#define HEAP_START 0xD0000000

static linear_addr_t* brk_base = (linear_addr_t*) (HEAP_START);
static linear_addr_t* brk_limit = (linear_addr_t*) (HEAP_START + BLOCK_SIZE);
static linear_addr_t* brk_hardLimit = (linear_addr_t*) (HEAP_START + 0x0FFFFFFF);
static ubyte_t has_mapped = 0;

#define UNUSED(x) x=x

/**
 * Move boundary by length
 */
static void* psbrk(udword_t length, ubyte_t reduce)
{
    if(!reduce)
    {
        // Step1: Move Limit
        linear_addr_t* prev_limit = brk_limit;
        brk_limit += (length << BLOCK_BITS);

        if(brk_limit >= brk_hardLimit) return (void*)0xFFFFFFFF;

        // Step2: Allocate Pages
        for(linear_addr_t* current_page = prev_limit; current_page < brk_limit; current_page += BLOCK_SIZE)
        {
            vmm_map_address(current_page, pmalloc(1), PAGE_PRESENT | PAGE_RW);
        }

        // Step3: Return beginning of space
        return (void*) prev_limit;
    }
    else if(reduce)
    {
        // Step1: Move Limit
        linear_addr_t* prev_limit = brk_limit;
        brk_limit -= (length << BLOCK_BITS);

        if(brk_limit >= brk_hardLimit) return (void*)0xFFFFFFFF;

        // Step2: Allocate Pages
        for(linear_addr_t* current_page = prev_limit; current_page > brk_limit; current_page -= BLOCK_SIZE)
        {
            vmm_unmap_address(current_page);
        }

        // Step3: Return beginning of space
        return NULL;
    } else return (void*) 0xFFFFFFFF;
}

int liballoc_lock()
{
    asm("cli");
    return 0;
}

int liballoc_unlock()
{
    asm("sti");
    return 0;
}

void* liballoc_alloc(size_t length)
{
    if(!has_mapped)
    {
        vmm_map_address(brk_base, pmalloc(1), PAGE_PRESENT | PAGE_RW);
        has_mapped = 1;
    }
    return psbrk(length, 0);
}

int liballoc_free(void* addr, size_t length)
{
    if(!has_mapped)
    {
        vmm_map_address(brk_base, pmalloc(1), PAGE_PRESENT | PAGE_RW);
        has_mapped = 1;
    }

    UNUSED(addr);

    return (linear_addr_t)psbrk(length, 1) != 0xFFFFFFFF;
}
