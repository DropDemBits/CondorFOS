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

#ifndef IO_H
#define IO_H

inline uint8_t inb(uint16_t port)
{
    uint8_t data = 0;
    asm volatile ("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

inline uint16_t inw(uint16_t port)
{
    uint16_t data = 0;
    asm volatile ("inw %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

inline uint32_t inl(uint16_t port)
{
    uint32_t data = 0;
    asm volatile ("inl %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

inline void outb(uint16_t port, uint8_t data)
{
    asm volatile ("outb %0, %1" :: "a"(data), "Nd"(port));
}

inline void outw(uint16_t port, uint16_t data)
{
    asm volatile ("outw %0, %1" :: "a"(data), "Nd"(port));
}

inline void outl(uint16_t port, uint32_t data)
{
    asm volatile ("outl %0, %1" :: "a"(data), "Nd"(port));
}

inline void io_wait(void)
{
    asm volatile ("outb %al, $0xE9");
}

#endif
