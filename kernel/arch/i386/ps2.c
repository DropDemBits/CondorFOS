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
#include <kernel/ps2.h>

#include <stdio.h>
#include <io.h>

#include <kernel/irq.h>
#include <kernel/klogger.h>
#include <kernel/hal.h>

ubyte_t devType[] = {DEV_TYPE_UNKNOWN, DEV_TYPE_UNKNOWN};
ubyte_t hasDEV2 = 0;

static ubyte_t waitACK(void)
{
    //Wait for it...
    while(!(inb(PS2_STT_CMD) & 0x1)) asm("pause");
    inb(PS2_STT_CMD);
    //Check for both 0xAA and ACK
    ubyte_t in = inb(PS2_DATA);
    if(in != 0xAA && in != PS2_ACK) return 1;
    if(in == 0xAA) inb(PS2_DATA);
    return 0;
}

static void detectDevice(int device)
{
    udword_t watch_dog = 0;
    ubyte_t byte0 = 0;
    ubyte_t byte1 = 0;
    ubyte_t byte0_store = 0;

    ps2_sendDataTo(device, 0xF5);
    ps2_sendDataTo(device, 0xF2);

    while(!(inb(PS2_STT_CMD) & 0x01) && watch_dog < 0xFFFF) {
        watch_dog++;
        asm("pause");
    }
    byte0_store = ps2_readDataFrom(device);
    watch_dog = 0;

    while(!(inb(PS2_STT_CMD) & 0x01) && watch_dog < 0xFFFF) {
        watch_dog++;
        asm("pause");
    }
    byte0 = ps2_readDataFrom(device);

    if(byte0_store != byte0 && byte0_store != PS2_ACK) byte0 = byte0_store;

    watch_dog = 0;
    while(!(inb(PS2_STT_CMD) & 0x01) && watch_dog < 0xFFFF) {
        watch_dog++;
        asm("pause");
    }
    byte1 = ps2_readDataFrom(device);

    while(inb(PS2_STT_CMD) & 0x1) inb(PS2_DATA);

    if(byte0 == 0x00 && byte1 == 0x00) devType[device] = DEV_TYPE_PS2_MOUSE;
    else if(byte0 == 0x03 && byte1 == 0x03) devType[device] = DEV_TYPE_MOUSE_SCR;
    else if(byte0 == 0x04 && byte1 == 0x04) devType[device] = DEV_TYPE_MOUSE_5B;
    else if(byte0 == 0xAB && byte1 == 0xC1) devType[device] = DEV_TYPE_MF2KBD_TRANS;
    else if(byte0 == 0xAB && byte1 == 0x41) devType[device] = DEV_TYPE_MF2KBD_TRANS;
    else if(byte0 == 0xAB && byte1 == 0x83) devType[device] = DEV_TYPE_MF2KBD;
    else if(byte0 == PS2_ACK && byte1 == PS2_ACK) devType[device] = DEV_TYPE_ATKBD_TRANS;
    else {
        printf("Unknown Response: %x, %x\n", byte0, byte1);
        devType[device] = DEV_TYPE_UNKNOWN;
    }
}

ubyte_t ps2_init(void)
{
    ubyte_t usable_channels = 0;
    ubyte_t config = 0;
    ubyte_t test_resp = 0;
    ubyte_t check_resp = 0;

    //Disable devices
    outb(PS2_STT_CMD, 0xAD);
    outb(PS2_STT_CMD, 0xA7);

    //Clear output buffer
    while(inb(PS2_STT_CMD) & 0x1) inb(PS2_DATA);

    //Read config byte, then send back
    outb(PS2_STT_CMD, 0x20);
    config = inb(PS2_DATA);
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
    test_resp = inb(PS2_DATA);
    if(test_resp != 0x55) {
        if(test_resp == 0xFC) return RET_FAIL;
        else {
            //Some undocumented response, report it
            printf("%#x, ", test_resp & 0xFF);
            return RET_FAIL | RET_NODEV;
        }
    }

    //Double check if this is actually a dual port controller
    if(hasDEV2) {
        //Enable 2nd port, then read config byte
        outb(PS2_STT_CMD, 0xA8);
        outb(PS2_STT_CMD, 0x20);
        config = inb(PS2_DATA);
        if((config & 0x20) == 0) {
            //No 2nd port, clear hasDEV2
            hasDEV2 = 0;
        }
        outb(PS2_STT_CMD, 0xA7);
    }

    //Test ports
    usable_channels = 0x1 | (hasDEV2 >> 4);

    outb(PS2_STT_CMD, 0xAB);
    check_resp = inb(PS2_DATA);
    if(check_resp != 0) usable_channels &= ~0x01;

    if(hasDEV2) {
        outb(PS2_STT_CMD, 0xA9);
        check_resp = inb(PS2_DATA);
        if(check_resp != 0) {
            usable_channels &= ~0x02;
            hasDEV2 = 0;
        } else hasDEV2 = 1;
    }

    if(!usable_channels) return RET_FAIL | RET_NODEV;

    //Enable and reset devices
    outb(PS2_STT_CMD, 0x20);
    config = inb(PS2_DATA);

    //Port1
    if(usable_channels & 0x1) {
        outb(PS2_STT_CMD, 0xAE);
        outb(PS2_DATA, 0xFF);
        if(waitACK()) usable_channels &= ~0x01;
        else config |= 0x01;
    }

    //Port2
    if(usable_channels & 0x2) {
        outb(PS2_STT_CMD, 0xA8);
        outb(PS2_STT_CMD, 0xD4);
        outb(PS2_DATA, 0xFF);
        if(waitACK()) usable_channels &= ~0x02;
        else config |= 0x02;
    }

    if(!usable_channels) return RET_FAIL | RET_NODEV;

    detectDevice(DEV1);
    if(hasDEV2) detectDevice(DEV2);

    if(devType[DEV1] == DEV_INV) config &= ~0x01;
    else config &= ~0x10;

    if(devType[DEV2] == DEV_INV) config &= ~0x02;
    else config &= ~0x20;

    outb(PS2_STT_CMD, 0x60);
    outb(PS2_DATA, config);

    outb(PS2_STT_CMD, 0x20);
    printf("PS/2 INFO: (DEVICE1: %d, ", devType[DEV1]);
    printf("DEVICE2: %d, ", devType[DEV2]);
    printf("CONFIG: %x), ", inb(PS2_DATA) & 0xFF);

    ps2_clearBuffer();
    return RET_SUCCESS;
}

void ps2_handleDevice(int device, irq_t func)
{
    if(device == DEV2) {
        if(!hasDEV2 || device == DEV_INV) return;
        irq_addISR(12, func);
    }
    irq_addISR(1, func);
}

int ps2_sendDataTo(int device, ubyte_t data)
{
    if((device == DEV2 && !hasDEV2) || device == DEV_INV) return 2;
    uword_t watch_dog = 0;
    while((inb(PS2_STT_CMD) & 0x2)) {
        watch_dog++;
        if(watch_dog > 1000) break;
    }
    if(watch_dog > 1000) return 1;

    if(device == DEV2) outb(PS2_STT_CMD, 0xD4);
    outb(PS2_DATA, data);
    waitACK();
    return 0;
}

ubyte_t ps2_readDataFrom(int device)
{
    if((device == DEV2 && !hasDEV2) || device == DEV_INV) return 0x00;
    return inb(PS2_DATA);
}

void ps2_clearBuffer()
{
    while(inb(PS2_STT_CMD) & 0x1) inb(PS2_DATA);
}

ubyte_t ps2_getType(int device)
{
    if(device == DEV_INV || device > DEV2) return DEV_TYPE_UNKNOWN;
    return devType[device];
}

static int getKeyboardDev()
{
    if(devType[1] == DEV_TYPE_ATKBD_TRANS || devType[1] == DEV_TYPE_MF2KBD  || devType[1] == DEV_TYPE_MF2KBD_TRANS)
        return DEV2;
    else if(devType[0] != DEV_TYPE_UNKNOWN) return DEV1;
    return DEV_INV;
}

static int getMouseDev()
{
    if(devType[1] == DEV_TYPE_PS2_MOUSE || devType[1] == DEV_TYPE_MOUSE_SCR  || devType[1] == DEV_TYPE_MOUSE_SCR)
        return DEV2;
    else if(devType[0] != DEV_TYPE_UNKNOWN) return DEV1;
    return DEV_INV;
}

int ps2_getDevHID(int hid)
{
    switch(hid) {
        case HID_KEYBOARD: return getKeyboardDev();
        case HID_MOUSE: return getMouseDev();
        default: return DEV_INV;
    }
}
