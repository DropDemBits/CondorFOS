#include <stdint.h>

#ifndef PIT_H
#define PIT_H

#if 0
#define MAIN_FRQ 11931892.0f
#define MILLI_INTERVAL (int)((float)(MAIN_FRQ/11931892.0f)*1000.0f)
#else
#define MAIN_FRQ (1193)
#define MILLI_INTERVAL (int)((float)(MAIN_FRQ/1193)*1000)
#endif

#define PIT_COUNTER0 0x40
#define PIT_COUNTER1 0x41
#define PIT_COUNTER2 0x42
#define PIT_COMMAND  0x43

void pit_init();
void pit_createCounter(uint32_t frequencey, uint8_t counter, uint8_t mode);
uint32_t pit_getTicks();
uint32_t pit_getMillis();
void pit_tsleep(uint32_t ticks);
void pit_sleep(uint32_t millis);

#endif
