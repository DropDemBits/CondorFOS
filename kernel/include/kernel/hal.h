#include <condor.h>

#ifndef _HAL_H_
#define _HAL_H_

//Controller Defines
#define HID_KEYBOARD 0
#define HID_MOUSE 1

void hal_init();

/**===================================================**\
|*     Hardware abstraction for the PIC and the APIC   *|
\**===================================================**/

void hal_initPIC(ubyte_t irqBase);

udword_t ic_readReg();

void ic_writeReg();

void ic_ack(ubyte_t irq);

void ic_unmaskIRQ(uword_t irq);

void ic_maskIRQ(uword_t irq);

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
