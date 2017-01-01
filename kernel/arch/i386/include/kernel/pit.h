#include <stdint.h>

#ifndef PIT_H
#   ifdef USE_HPET
#       error Cannot use both HPET and PIT
#   endif
#define PIT_H

#define PIT_COUNTER0 0x40
#define PIT_COUNTER1 0x41
#define PIT_COUNTER2 0x42
#define PIT_COMMAND  0x43

void pit_init();
void pit_createCounter(uint32_t frequencey, uint8_t counter, uint8_t mode);
uint32_t pit_getTicks();
void pit_sleep(uint32_t ticks);

#endif
