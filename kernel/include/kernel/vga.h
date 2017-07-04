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

#ifndef _VGA_H
#define _VGA_H

#define VGA_MEMORY 0xC00B8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

enum VGA_color_t {
    VGA_BLACK,
    VGA_BLUE,
    VGA_GREEN,
    VGA_CYAN,
    VGA_RED,
    VGA_MAGENTA,
    VGA_BROWN,
    VGA_GREY,
    VGA_DARK_GREY,
    VGA_LIGHT_BLUE,
    VGA_LIGHT_GREEN,
    VGA_LIGHT_CYAN,
    VGA_LIGHT_RED,
    VGA_LIGHT_MAGENTA,
    VGA_YELLOW,
    VGA_WHITE,
};
typedef enum VGA_color_t VGA_color;


/**====================================**\
|**          Public Functions          **|
\**====================================**/

inline uint8_t vga_makeColor(uint8_t foreground, uint8_t background)
{
    return (uint8_t) background << 4 | foreground;
}

inline uint16_t vga_makeEntry(unsigned char uc, const uint8_t color)
{
    return (uint16_t) uc | color << 8;
}

#endif
