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

#include <condor.h>

#ifndef _HAL_H_
#define _HAL_H_

//Controller Defines
#define HID_KEYBOARD 0
#define HID_MOUSE 1

/**===================================================**\
|*                 General Abstractions                *|
\**===================================================**/

void hal_init();

void hal_disableInterrupts();

void hal_enableInterrupts();

/**===================================================**\
|*     Hardware abstraction for the PIC and the APIC   *|
\**===================================================**/

void hal_initPIC(ubyte_t irqBase);

udword_t ic_readReg();

void ic_writeReg();

void ic_ack(uint8_t irq);

void ic_unmaskIRQ(uint8_t irq);

void ic_maskIRQ(uint8_t irq);

int ic_checkSpurious(uint8_t irq);

/**===================================================**\
|*     Hardware abstraction for the PIT and the HPET   *|
\**===================================================**/

void hal_initTimer();

void timer_init();

void timer_createCounter(udword_t frequencey, ubyte_t counter, ubyte_t mode);

udword_t timer_getTicks();

udword_t timer_getMillis();

void timer_sleep(udword_t millis);

void timer_tsleep(udword_t ticks);

/**===================================================**\
|*   Hardware abstraction for the device controllers   *|
\**===================================================**/

void hal_initController();

void controller_handleDevice(int device, uqword_t func);

int controller_sendDataTo(int device, ubyte_t data);

ubyte_t controller_readDataFrom(int device);

ubyte_t controller_getType(int device);

int controller_getDevHID(int hid);

#endif
