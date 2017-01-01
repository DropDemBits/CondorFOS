/**===================================================**\
|* Hardware abstraction layer for the PIC and the APIC *|
\**===================================================**/
#include <stdint.h>
#ifndef PIC_HAL_H_FUNCS
#define PIC_HAL_H_FUNCS

/* Initializes a timer (Does the appropriate things) */
void ic_init(uint16_t irqBase);

void ic_getBase();

void ic_ack(uint16_t irq);

void ic_maskIRQ(uint16_t irq);
void ic_clearIRQ(uint16_t irq);

/* The addr is ignored for the PIC */
uint32_t ic_read(void* addr, uint32_t reg);
/* The addr is ignored for the PIC */
void ic_write(void *addr, uint32_t reg, uint32_t value);

#endif

#ifndef SING_INC
#define SING_INC
#ifndef USE_APIC
#   ifndef PIC_H
#       include <kernel/pic.h>
#   endif
#else
#   ifndef APIC_H
#       include <kernel/apic.h>
#   endif
#endif
#endif
