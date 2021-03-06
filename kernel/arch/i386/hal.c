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
#include <kernel/hal.h>

#include <kernel/pic.h>
#include <kernel/apic.h>
#include <kernel/pit.h>
#include <kernel/hpet.h>
#include <kernel/ps2.h>

/**===================================================**\
|*                 General Abstractions                *|
\**===================================================**/

static ubyte_t were_intrs_enabled = 0;

void hal_init()
{

}

void hal_disableInterrupts()
{
    asm volatile("pushf\n\t"
        "popl %%eax\n\t"
        "andl $0x200, %%eax\n\t"
        "movl %%eax, (%0)\n\t"
        "cli\n\t"
        :"=m"(were_intrs_enabled)::"eax");
}

void hal_enableInterrupts()
{
    asm volatile("pushf\n\t"
        "popl %%eax\n\t"
        "andl $0x200, %%eax\n\t"
        "movl %%eax, (%0)\n\t"
        "sti\n\t"
        :"=m"(were_intrs_enabled)::"eax");
}

void hal_restoreInterrupts()
{
    if(were_intrs_enabled) {
        asm("sti");
    }
    else {
        asm("cli");
    }
}

/**===================================================**\
|*     Hardware abstraction for the PIC and the APIC   *|
\**===================================================**/

void hal_initPIC(ubyte_t irqBase)
{
    pic_init(irqBase, irqBase+8);
}

udword_t ic_readReg(udword_t reg)
{
    return pic_read(reg);
}

void ic_writeReg(udword_t reg, udword_t value)
{
    pic_write(reg, value);
}

void ic_ack(ubyte_t irq)
{
    pic_ack(irq);
}

void ic_unmaskIRQ(uint8_t irq)
{
    pic_unmaskIRQ(irq);
}

void ic_maskIRQ(uint8_t irq)
{
    pic_maskIRQ(irq);
}

int ic_checkSpurious(uint8_t irq)
{
    return pic_checkSpurious(irq);
}

/**===================================================**\
|*     Hardware abstraction for the PIT and the HPET   *|
\**===================================================**/

void hal_initTimer()
{
    pit_init();
}

void timer_createCounter(udword_t frequency, ubyte_t counter, ubyte_t mode)
{
    pit_createCounter(frequency, counter, mode);
}

void timer_writeReload(uint16_t value, uint8_t counter)
{
    pit_writeReload(value, counter);
}

/**===================================================**\
|*   Hardware abstraction for the device controllers   *|
\**===================================================**/

void hal_initController()
{
    printf("RET: %x\n", ps2_init() & 0xFF);
}

void controller_handleDevice(int device, irq_t func)
{
    ps2_handleDevice(device, func);
}

int controller_sendDataTo(int device, ubyte_t data)
{
    return ps2_sendDataTo(device, data);
}

ubyte_t controller_readDataFrom(int device)
{
    return ps2_readDataFrom(device);
}

ubyte_t controller_getType(int device)
{
    return ps2_getType(device);
}

int controller_getDevHID(int hid)
{
    return ps2_getDevHID(hid);
}
