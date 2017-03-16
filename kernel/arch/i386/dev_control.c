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

ubyte_t devType[] = {DEV_TYPE_UNKNOWN, DEV_TYPE_UNKNOWN};
ubyte_t hasDEV2 = 0;

static ubyte_t waitACK(void)
{
    //Wait for it...
    while(!(inb(PS2_STT_CMD) ^ 0x01)) asm("pause");

    //Check for both 0xAA and 0xFA
    ubyte_t in = inb(PS2_DATA);
    if(in != 0xAA || in != 0xFA) return 1;
    if(in == 0xAA) inb(PS2_DATA);
    return 0;
}

static void detectDevice(int device)
{
    //Disable scanning for device
    outb(PS2_STT_CMD, 0xAD);
    controller_sendDataTo(device, 0xF5);
    controller_sendDataTo(device, 0xF2);
    while(!(inb(PS2_STT_CMD) ^ 0x01)) asm("pause");
    inb(PS2_DATA);
    ubyte_t watch_dog = 0;
    while(!(inb(PS2_STT_CMD) ^ 0x01) && watch_dog < 255)
    {
        ++watch_dog;
        asm("pause");
    }
    ubyte_t byte0 = controller_readDataFrom(device);
    watch_dog = 0;
    while(!(inb(PS2_STT_CMD) ^ 0x01) && watch_dog < 255)
    {
        ++watch_dog;
        asm("pause");
    }
    ubyte_t byte1 = controller_readDataFrom(device);

    if(byte0 == 0x00 && byte1 == 0x00) devType[device] = DEV_TYPE_PS2_MOUSE;
    else if(byte0 == 0x03 && byte1 == 0x03) devType[device] = DEV_TYPE_MOUSE_SCR;
    else if(byte0 == 0x04 && byte1 == 0x04) devType[device] = DEV_TYPE_MOUSE_5B;
    else if(byte0 == 0xAB && byte1 == 0xC1) devType[device] = DEV_TYPE_MF2KBD_TRANS;
    else if(byte0 == 0xAB && byte1 == 0x41) devType[device] = DEV_TYPE_MF2KBD_TRANS;
    else if(byte0 == 0xAB && byte1 == 0x83) devType[device] = DEV_TYPE_MF2KBD;
    else if(byte0 == 0xFA && byte1 == 0xFA) devType[device] = DEV_TYPE_ATKBD_TRANS;
    else devType[device] = DEV_TYPE_UNKNOWN;
}

ubyte_t controller_init(void)
{
    //TODO: Make this less bulky

    //Disable devices
    outb(PS2_STT_CMD, 0xAD);
    outb(PS2_STT_CMD, 0xA7);

    //Clear output buffer
    while(inb(PS2_STT_CMD) & 0x1) inb(PS2_DATA);

    //Read config byte, then send back
    outb(PS2_STT_CMD, 0x20);
    ubyte_t config = inb(PS2_DATA);
    outb(PS2_STT_CMD, 0x60);

    //Wait for buffer to be ready
    while(inb(PS2_STT_CMD) & 0x02) asm("pause");
    //Send back config byte
    outb(PS2_DATA, config & ~0x43);

    //Test for 2nd PS/2 port
    hasDEV2 = config ^ 0x20;

    //Initiate self test
    outb(PS2_STT_CMD, 0xAA);
    while(!(inb(PS2_STT_CMD) ^ 0x01)) asm("pause");
    ubyte_t test_resp = inb(PS2_DATA);
    if(test_resp != 0x55)
    {
        if(test_resp == 0xFC) return RET_FAIL;
        else
        {
            //Some undocumented response, report it
            printf("%#x\n");
            kpanic("Unknown response");
        }
    }

    //Double check if this is actually a dual port controller
    if(hasDEV2)
    {
        //Enable 2nd port, then read
        outb(PS2_STT_CMD, 0xA8);
        outb(PS2_STT_CMD, 0x20);
        config = inb(PS2_DATA);
        if(config & 0x20) outb(PS2_STT_CMD, 0xA7);
        else
        {
            //No 2nd port, disable and clear hasDEV2
            outb(PS2_STT_CMD, 0xA7);
            hasDEV2 = 0;
        }
    }

    //Test ports
    ubyte_t usable_channels = 0x1 | (hasDEV2 >> 4);

    outb(PS2_STT_CMD, 0xAB);
    ubyte_t check_resp = inb(PS2_DATA);
    if(check_resp != 0) usable_channels &= ~0x01;

    if(hasDEV2)
    {
        outb(PS2_STT_CMD, 0xA9);
        ubyte_t check_resp = inb(PS2_DATA);
        if(check_resp != 0)
        {
            usable_channels &= ~0x02;
            hasDEV2 = 0;
        } else hasDEV2 = 1;
    }

    if(!usable_channels) return RET_FAIL | RET_NODEV;

    //Enable and reset devices
    outb(PS2_STT_CMD, 0x20);
    config = inb(PS2_DATA);
    //Port1
    if(usable_channels & 0x1)
    {
        outb(PS2_STT_CMD, 0xAE);
        outb(PS2_DATA, 0xFF);
        if(!waitACK()) usable_channels &= ~0x01;
        config |= 0x01;
    }

    //Port2
    if(usable_channels & 0x2)
    {
        outb(PS2_STT_CMD, 0xA8);
        outb(PS2_STT_CMD, 0xD4);
        outb(PS2_DATA, 0xFF);
        if(!waitACK()) usable_channels &= ~0x02;
        else config |= 0x02;
    }

    if(!usable_channels) return RET_FAIL | RET_NODEV;

    printf("%#x", config);

    outb(PS2_STT_CMD, 0x60);
    outb(PS2_DATA, config);

    detectDevice(DEV1);
    if(hasDEV2) detectDevice(DEV2);

    //Clear buffer
    while(inb(PS2_STT_CMD) & 0x01) inb(PS2_DATA);
    controller_clearBuffer();
    return RET_SUCCESS;
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
    while((inb(PS2_STT_CMD) & 0x2))
    {
        watch_dog++;
        if(watch_dog > 1000) break;
    }
    if(watch_dog > 1000) return 1;

    if(device == DEV2) outb(PS2_STT_CMD, 0xD4);
    outb(PS2_DATA, data);
    waitACK();
    return 0;
}

ubyte_t controller_readDataFrom(int device)
{
    if((device == DEV2 && !hasDEV2) || device == DEV_INV) return 0x00;
    return inb(PS2_DATA);
}

void controller_clearBuffer()
{
    while(inb(PS2_STT_CMD) & 0x1) inb(PS2_DATA);
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
