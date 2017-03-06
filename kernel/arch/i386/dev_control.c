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
#include <stdio.h>
#include <io.h>

#include <kernel/idt.h>
#include <kernel/timer_hal.h>
#include <kernel/klogger.h>

int devType[] = {DEV_TYPE_UNKNOWN, DEV_TYPE_UNKNOWN};
int hasDEV2 = 0;

void controller_init(void)
{
    outb(PS2_STT_CMD, 0xAD);
    outb(PS2_STT_CMD, 0xA7);
    while(inb(PS2_DATA) && (inb(PS2_STT_CMD) & 0x1)) asm("pause");
    outb(PS2_STT_CMD, 0x20);
    ubyte_t cont_conf = inb(PS2_DATA);
    ubyte_t channels = cont_conf & ~0x20;
    cont_conf = cont_conf & 0x23;
    outb(PS2_STT_CMD, 0x60);
    outb(PS2_DATA, cont_conf);
    again:    
    outb(PS2_STT_CMD, 0xAA);
    while((inb(PS2_STT_CMD) & 0x1) == 0) asm("pause");
    ubyte_t resp = inb(PS2_DATA);
    if(resp != 0x55)
    {
        if(resp == 0xFE) goto again;
        printf("%#x\n", resp);
        kpanic("Failed to initialize PS/2 controller");
    }
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
        else
        {
            channels |= 0x8;
            hasDEV2 = 1;
        }
    }
    if(!channels) kpanic("Something is wrong with the PS/2 Ports...");
    
    cont_conf = inb(PS2_DATA);
    
    ubyte_t b0 = 0, b1 = 0, b2 = 0;
    
    if(channels & 0x1)
    {
        outb(PS2_STT_CMD, 0xAE);
        outb(PS2_DATA, 0xFF);
        while((inb(PS2_STT_CMD) & 0x1) == 0) asm("pause");
        ubyte_t first_byte = inb(PS2_DATA);
        if(first_byte != 0xAA && first_byte != 0xFA)
        {
            channels &= ~0x1;
            goto skip;
        }
        outb(PS2_DATA, 0xF5);
        while((inb(PS2_STT_CMD) & 0x1) == 0) asm("pause");
        ubyte_t in = inb(PS2_DATA);
        if(in != 0xFA && in != 0xAA) goto skip;
        cont_conf |= 0x1;
        
        //ID device
        outb(PS2_STT_CMD, 0xAD);
        while(inb(PS2_DATA) != 0xFA);
        while((inb(PS2_STT_CMD) & 0x1)) inb(PS2_DATA);
        outb(PS2_DATA, 0xF2);
        while((inb(PS2_STT_CMD) & 0x1) == 0) asm("pause");
        ubyte_t bACK = inb(PS2_DATA);
        if(bACK != 0xFA)
        {
            printf("%#x\n", bACK);
            kpanic("NON_ACK");
        }
        
        uword_t watch_dog = 0;
        while((inb(PS2_STT_CMD) & 0x1) == 0)
        {
            if(watch_dog >= 2000)
            {
                watch_dog = 0xFFFF;
                break;
            }
            asm("pause");
        }
        if(watch_dog != 0xFFFF) b0 = inb(PS2_DATA);
        else b0 = 0xBA;
        watch_dog = 0;
        
        while((inb(PS2_STT_CMD) & 0x1) == 0)
        {
            if(watch_dog >= 2000)
            {
                watch_dog = 0xFFFF;
                break;
            }
            asm("pause");
        }
        if(watch_dog != 0xFFFF) b1 = inb(PS2_DATA);
        else b1 = 0xBA;
        
        if(b0 == 0xBA && b1 == 0xBA) devType[0] = DEV_TYPE_ATKBD_TRANS;
        else if(b0 == 0xBA && b1 == 0xBA) devType[0] = DEV_TYPE_PS2_MOUSE;
        else if(b0 == 0xBA && b1 == 0xBA) devType[0] = DEV_TYPE_MOUSE_SCR;
        else if(b0 == 0xBA && b1 == 0xBA) devType[0] = DEV_TYPE_MOUSE_5B;
        else if(b0 == 0xAB && (b1 == 0x41 || b1 == 0xC1)) devType[0] = DEV_TYPE_MF2KBD_TRANS;
        else if(b0 == 0xAB && b1 == 0x83) devType[0] = DEV_TYPE_MF2KBD;
        else devType[0] = DEV_TYPE_UNKNOWN;
        
        while(inb(PS2_DATA) && (inb(PS2_STT_CMD) & 0x1)) asm("pause");
    }
    
    skip:
    if(channels & 0x8 && hasDEV2)
    {
        outb(PS2_STT_CMD, 0xA8);
        outb(PS2_DATA, 0xD4);
        outb(PS2_DATA, 0xFF);
        while((inb(PS2_STT_CMD) & 0x1) == 0) asm("pause");
        ubyte_t first_byte = inb(PS2_DATA);
        if(first_byte != 0xAA && first_byte != 0xFA)
        {
            channels &= ~0x8;
            goto end;
        }
        outb(PS2_DATA, 0xD4);
        outb(PS2_DATA, 0xF5);
        while((inb(PS2_STT_CMD) & 0x1) == 0) asm("pause");
        ubyte_t in = inb(PS2_DATA);
        if(in != 0xFA) goto skip;
        
        //ID device
        outb(PS2_STT_CMD, 0xD3);
        outb(PS2_DATA, 0xBA);
        outb(PS2_STT_CMD, 0xD3);
        outb(PS2_DATA, 0xBA);
        outb(PS2_STT_CMD, 0xD3);
        outb(PS2_DATA, 0xBA);
        
        outb(PS2_DATA, 0xF2);
        
        sleep(MILLI_INTERVAL*500);
        b0 = inb(PS2_DATA);
        b1 = inb(PS2_DATA);
        b2 = inb(PS2_DATA);
        if(b0 == 0xBA)
        {
            if(b1 == 0xBA && b2 == 0xBA) goto here;
            else if(b1 == 0xBA && b2 == 0x00) devType[1] = DEV_TYPE_PS2_MOUSE;
            else if(b1 == 0xBA && b2 == 0x00) devType[1] = DEV_TYPE_MOUSE_SCR;
            else if(b1 == 0xBA && b2 == 0x00) devType[1] = DEV_TYPE_MOUSE_5B;
            else if(b1 == 0xAB && (b2 == 0x41 || b2 == 0xC1)) goto here;
            else if(b1 == 0xAB && b2 == 0x83) devType[1] = DEV_TYPE_MF2KBD;
            else devType[1] = DEV_TYPE_UNKNOWN;
        } else
        {
            here:
            devType[1] = DEV_TYPE_UNKNOWN;
            logErro("PS/2 DEV2 Detect Fail\n");
        }
        cont_conf |= 0x2;
    }
    
    end:
    outb(PS2_STT_CMD, 0x60);
    outb(PS2_DATA, cont_conf);
    if(!channels) logWarn("No input devices detected");
}

void controller_handleDevice(int device, uqword_t func)
{
    if(device == DEV2)
    {
        if(!hasDEV2 || device == DEV_INV) return;
        idt_addISR(IRQ12, func);
    }
    idt_addISR(IRQ1, func);
}

int controller_sendDataTo(int device, ubyte_t data)
{
    if((device == DEV2 && !hasDEV2) || device == DEV_INV) return 2;
    uword_t watch_dog = 0;
    while((inb(PS2_STT_CMD) & 0x1) == 0)
    {
        watch_dog++;
        if(watch_dog > 1000) break;
    }
    if(watch_dog > 1000) return 1;
    
    if(device == DEV2) outb(PS2_STT_CMD, 0xD4);
    outb(PS2_DATA, data);
    while(inb(PS2_DATA) != 0xFA) asm("pause");
    return 0;
}

ubyte_t controller_readDataFrom(int device)
{
    if((device == DEV2 && !hasDEV2) || device == DEV_INV) return 0x00;
    return inb(PS2_DATA);
}

ubyte_t controller_getType(int device)
{
    if(device == DEV_INV || device > DEV2) return DEV_TYPE_UNKNOWN;
    return devType[device];
}

int controller_getKeyboardDev()
{
    if(devType[1] == DEV_TYPE_ATKBD_TRANS || devType[1] == DEV_TYPE_MF2KBD  || devType[1] == DEV_TYPE_MF2KBD_TRANS)
        return DEV2;
    else if(devType[0] != DEV_TYPE_UNKNOWN) return DEV1;
    return DEV_INV;
}

int controller_getMouseDev()
{
    if(devType[1] == DEV_TYPE_PS2_MOUSE || devType[1] == DEV_TYPE_MOUSE_SCR  || devType[1] == DEV_TYPE_MOUSE_SCR)
        return DEV2;
    else if(devType[0] != DEV_TYPE_UNKNOWN) return DEV1;
    return DEV_INV;
}
