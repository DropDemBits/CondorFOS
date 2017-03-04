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

#include <kernel/dev_control.h>
#include <kernel/timer_hal.h>

int hasDEV2 = 0;

void controller_init(void)
{
    outb(PS2_STT_CMD, 0xAD);
    outb(PS2_STT_CMD, 0xA7);
    while(inb(PS2_DATA)) asm("hlt");
    outb(PS2_STT_CMD, 0x20);
    ubyte_t cont_conf = inb(PS2_DATA);
    cont_conf = cont_conf ^ 0x43;
    ubyte_t channels = cont_conf & 0x20;
    outb(PS2_DATA, cont_conf);
    outb(PS2_STT_CMD, 0x60);
    outb(PS2_STT_CMD, 0xAA);
    while(inb(PS2_STT_CMD) & 0x1 == 0) asm("hlt");
    if(inb(PS2_DATA) != 0x55) kpanic("Failed to initialize PS/2 controller");
    if(channels)
    {
        outb(PS2_STT_CMD, 0xA8);
        outb(PS2_STT_CMD, 0x20);
        cont_conf = inb(PS2_DATA) & 0x20;
        if(!cont_conf) channels = 0;
        else channels = 0x8;
        outb(PS2_STT_CMD, 0xA7);
    }
    outb(PS2_STT_CMD, 0xAB);
    if(!inb(PS2_STT_CMD)) channels |= 0;
    else channels |= 0x1;
    if(channels & 0x8) 
    {
        outb(PS2_STT_CMD, 0xA9);
        if(!inb(PS2_STT_CMD)) channels &= ~0x8;
        else channels |= 0x8;
    }
    if(!channels) kpanic("Something is wrong with the PS/2 Ports...");
    
    cont_conf = inb(PS2_DATA);
    
    if(channels & 0x1)
    {
        outb(PS2_STT_CMD, 0xAE);
        outb(PS2_STT_CMD, 0xD2);
        outb(PS2_STT_CMD, 0xFF);
        if(inb(PS2_DATA) == 0xFC)
        {
            channels &= ~0x1;
            goto skip;
        }
        outb(PS2_STT_CMD, 0xD2);
        outb(PS2_STT_CMD, 0xF5);
        while(inb(PS2_DATA) != 0xFA) asm("hlt");
        cont_conf |= 0x1;
    }
    
    skip:
    if(channels & 0x8)
    {
        outb(PS2_STT_CMD, 0xA8);
        outb(PS2_STT_CMD, 0xD2);
        outb(PS2_STT_CMD, 0xFF);
        if(inb(PS2_DATA) == 0xFC)
        {
            channels &= ~0x8;
            goto skip;
        }
        outb(PS2_STT_CMD, 0xD2);
        outb(PS2_STT_CMD, 0xF5);
        cont_conf |= 0x2;
    }
    
    end:
    outb(PS2_DATA, cont_conf);
}

void controller_handleDevice(int device, uqword_t func)
{
    if(device == DEV2 && !hasDEV2);
}

int controller_sendDataTo(int device, ubyte_t data)
{
    
}

ubyte_t controller_readDataFrom(int device, ubyte_t data)
{
    
}

int controller_getKeyboardDev()
{
    
}

int controller_getMouseDev()
{
    
}
