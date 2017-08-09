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

#include <stdint.h>

#ifndef PIT_H
#define PIT_H

#define MAIN_FRQ (150)

#define PIT_COUNTER0 0x40
#define PIT_COUNTER1 0x41
#define PIT_COUNTER2 0x42
#define PIT_COMMAND  0x43

#define PIT_SELECT_0 0b00000000
#define PIT_SELECT_1 0b01000000
#define PIT_SELECT_2 0b10000000
#define PIT_READBACK 0b11000000

#define PIT_ACCESS_MODE_LATCH 0b00000000
#define PIT_ACCESS_MODE_LO    0b00010000
#define PIT_ACCESS_MODE_HI    0b00100000
#define PIT_ACCESS_MODE_LO_HI 0b00110000

#define PIT_MODE_TRIGGER_ON_TERM_COUNT 0b00000000
#define PIT_MODE_ONE_SHOT              0b00000010
#define PIT_MODE_RATE_GENERATOR        0b00000100
#define PIT_MODE_SQUARE_WAVE_GENERATOR 0b00000110
#define PIT_MODE_SOFTWARE_STROBE       0b00001000
#define PIT_MODE_HARDWARE_STROBE       0b00001010

#define PIT_COUNT_BINARY 0b00000000
#define PIT_COUNT_BCD    0b00000001

void pit_init();
void pit_createCounter(uint32_t frequency, uint8_t counter, uint8_t mode);
void pit_writeReload(uint16_t value, uint8_t counter);

#endif
