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
#include <kernel/vga.h>

#ifndef _TTY_H
#define _TTY_H

typedef struct {
    uint8_t should_update_cursor : 1;
    uint8_t monochrome : 1;
    uint8_t indexed_color : 1;
} tflags_t;

typedef struct {
    uint8_t foreground : 4;
    uint8_t background : 4;
} tcolour_index_t;

typedef struct {
    uint8_t term_char;
    uint8_t term_colour;
} tchar_t;

typedef struct {
    uint16_t width;
    uint16_t height;
    uint16_t column;
    uint16_t row;
    uint16_t column_store;
    uint16_t row_store;
    tcolour_index_t default_color;
    tflags_t flags;
    tchar_t* buffer;
} terminal_t;

/**
 * Clears the terminal
 */
void terminal_clear(void);

/**
 * Initializes the terminal
 */
void terminal_init(void);

/**
 * Moves the text cursor to the specified position
 * @param x The new x of the cursor
 * @param y The new y of the cursor
 */
void terminal_moveCursor(uint8_t x, uint8_t y);

/**
 * Prints out a coloured char onto the terminal
 * @param uc The char to print
 * @param color The color to print as (EGA colours only)
 */
void terminal_putchar_Color(const char uc, uint16_t color);

/**
 * Prints out a coloured string onto the terminal
 * @param string The string to print
 * @param color The color to print as (EGA colours only)
 */
void terminal_puts_Color(const char* string, uint16_t color);

/**
 * Prints out a char onto the terminal\n
 * The colour of the char is based on the current terminal colour
 * @param string The char to print
 */
void terminal_putchar(const char uc);

/**
 * Prints out a string onto the terminal\n
 * The colour of the string is based on the current terminal colour
 * @param string The string to print
 */
void terminal_puts(const char* string);

/**
 * Scrolls down the terminal
 */
void terminal_scroll(void);

/**
 * Sets the default colour of the terminal
 * @param fg The new foreground color
 * @param bg The new background color
 */
void terminal_setColor(uint8_t fg, uint8_t bg);

/**
 * Gets the current colour of the terminal
 * @return the current colour of the terminal
 */
uint16_t terminal_getColor(void);

void terminal_storePosition();

void terminal_restorePosition();

void terminal_set_shouldUpdateCursor(uint8_t value);

#endif
