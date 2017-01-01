#include <kernel/pic_hal.h>

#ifndef PIC_H
#   ifdef USE_APIC
#       error Cannot use both PIC and apic
#   endif
#define PIC_H

void pic_init(uint16_t irq0Base, uint16_t irq8Base);
void pic_getBase();
void pic_ack(uint16_t irq);
void pic_maskIRQ(uint16_t irq);
void pic_clearIRQ(uint16_t irq);
uint32_t pic_read(uint32_t reg);
void pic_write(uint32_t reg, uint32_t value);

#endif
