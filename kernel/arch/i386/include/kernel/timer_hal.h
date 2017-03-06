#include <stdint.h>

#ifndef TIMER_HAL_H
#define TIMER_HAL_H

void timer_init();
void timer_createCounter(uint32_t frequencey, uint8_t counter, uint8_t mode);
uint32_t timer_getTicks();
uint32_t timer_getMillis();
void sleep(uint32_t ticks);

#endif

#ifdef USE_HPET
#   ifndef HPET_H
#       include <hpet.h>
#   endif
#else
#   ifndef PIT_H
#       include <kernel/pit.h>
#   endif
#endif
