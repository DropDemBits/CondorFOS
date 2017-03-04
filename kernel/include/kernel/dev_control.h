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
 * File:   dev_control.h
 * Author: DropDemBits <r3usrlnd@gmail.com>
 *
 * Created on March 4, 2017, 10:15 AM
 */

#include <condor.h>

#ifndef DEV_CONTROL_H
#define DEV_CONTROL_H

#define DEV0 0
#define DEV1 1
#define DEV2 2
#define DEV3 3

#define PS2_DATA    0x60
#define PS2_STT_CMD 0x64
#define PS2_ACK     0xFA
#define PS2_RESEND  0xFE

void controller_init(void);

void controller_handleDevice(int device, uqword_t func);

int controller_sendDataTo(int device, ubyte_t data);

ubyte_t controller_readDataFrom(int device, ubyte_t data);

#endif /* DEV_CONTROL */

