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
 * 
 * condor.c: Implementation of condor.h
 */
#include <condor.h>
#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/klogger.h>

extern udword_t stack_bottom;

extern udword_t readCR0();
extern udword_t readCR2();
extern udword_t readCR3();
extern udword_t readCR4();

static char* getTable(int table)
{
    if(table & 4) return "LDT";
    else return "GDT";
}

void kexit(int status)
{
    if(!status)
        kpanic("Placeholder");
    else
        kpanic("Abnormal exit");
}

void kpanic(const char* message)
{
    logFErr(message);
    
    asm("cli");
    for(;;) asm("hlt");
}

void kputchar(const char c)
{
    terminal_putchar(c);
}

void kdump_useStack(uqword_t* rsp)
{
    udword_t* esp = (udword_t*)rsp;
    
    //Registers
    uint16_t cs =     *(esp+15) & 0xFFFF;
    uint16_t ds =     *(esp+ 3) & 0xFFFF;
    uint16_t es =     *(esp+ 2) & 0xFFFF;
    uint16_t fs =     *(esp+ 1) & 0xFFFF;
    uint16_t gs =     *(esp+ 0) & 0xFFFF;
    uint32_t eflags = *(esp+16) & 0xFFFF;
    uint32_t eip =    *(esp+14);
    
    //Dump Registers:
    printf("BEGIN DUMP:\n");
    printf("REGS: EAX: %#lx, EBX: %#lx, ECX: %#lx, EDX: %#lx\n",
            *(esp+11),*(esp+ 8),*(esp+10),*(esp+ 9));
    printf("ESP: %#lx, EBP: %#lx, ESI: %#lx, EDI: %#lx\n",
            *(esp+ 7),*(esp+ 6),*(esp+ 5),*(esp+ 4));
    printf("SEGMENT REGS: VALUE (INDEX|TABLE|RPL)\n");
    printf("CS: %x (%d|%s|%d)\n", cs, cs >> 4, getTable(cs), cs & 0x2);
    printf("DS: %x (%d|%s|%d)\n", ds, ds >> 4, getTable(ds), ds & 0x2);
    printf("ES: %x (%d|%s|%d)\n", es, es >> 4, getTable(es), es & 0x2);
    printf("FS: %x (%d|%s|%d)\n", fs, fs >> 4, getTable(fs), fs & 0x2);
    printf("GS: %x (%d|%s|%d)\n", gs, gs >> 4, getTable(gs), gs & 0x2);
    printf("EFLAGS: %#lx\n", eflags);
    printf("EIP: %#lx\n", eip);
    printf("CR0: %lx, CR2: %lx, CR3: %lx, CR4: %lx\n", readCR0(), readCR2(), readCR3(), readCR4());
}

void kdump_useRegs(uqword_t rip)
{
    //Registers
    uint16_t cs =     0;//__readReg(6);
    uint16_t ds =     0;//__readReg(7);
    uint16_t es =     0;//__readReg(8);
    uint16_t fs =     0;//__readReg(9);
    uint16_t gs =     0;//__readReg(10);
    uint32_t eflags = 0;//__readReg(11);
    
    uint32_t eax = 0;//__readReg(12);
    uint32_t ebx = 0;//__readReg(13);
    uint32_t ecx = 0;//__readReg(14);
    uint32_t edx = 0;//__readReg(15);
    uint32_t esp = 0;//__readReg(16);
    uint32_t ebp = 0;//__readReg(17);
    uint32_t esi = 0;//__readReg(18);
    uint32_t edi = 0;//__readReg(19);
    
    //Dump Registers:
    printf("BEGIN DUMP:\n");
    printf("REGS: EAX: %#lx, EBX: %#lx, ECX: %#lx, EDX: %#lx\n", eax, ebx, ecx, edx);
    printf("ESP: %#lx, EBP: %#lx, ESI: %#lx, EDI: %#lx\n", esp, ebp, esi, edi);
    printf("SEGMENT REGS: VALUE (INDEX|TABLE|RPL)\n");
    printf("CS: %x (%d|%s|%d)\n", cs, cs >> 4, getTable(cs), cs & 0x2);
    printf("DS: %x (%d|%s|%d)\n", ds, ds >> 4, getTable(ds), ds & 0x2);
    printf("ES: %x (%d|%s|%d)\n", es, es >> 4, getTable(es), es & 0x2);
    printf("FS: %x (%d|%s|%d)\n", fs, fs >> 4, getTable(fs), fs & 0x2);
    printf("GS: %x (%d|%s|%d)\n", gs, gs >> 4, getTable(gs), gs & 0x2);
    printf("EFLAGS: %#lx\n", eflags);
    printf("EIP: %#lx\n", (udword_t)rip);
    printf("CR0: %lx, CR2: %lx, CR3: %lx, CR4: %lx\n", readCR0(), readCR2(), readCR3(), readCR4());
    kdumpStack((uqword_t*)esp, stack_bottom);
}

void kdumpStack(uqword_t* rsp, udword_t ebp)
{
    udword_t* esp = (udword_t*) rsp;
    for(udword_t* ind = esp; ((udword_t)ind) > ebp; ind -= 4)
        printf("[STACK|0x%lx] [0x%lx]", ind, *ind);
}
