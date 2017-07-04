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

#ifndef PIC_H
#define PIC_H

void pic_init(uint8_t irq0Base, uint8_t irq8Base);
//void pic_getBase();
void pic_ack(uint8_t irq);
void pic_maskIRQ(uint8_t irq);
void pic_unmaskIRQ(uint8_t irq);
uint32_t pic_read(uint32_t reg);
void pic_write(uint32_t reg, uint32_t value);
int pic_checkSpurious(uint8_t irq);

#endif
