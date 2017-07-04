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

#include <kernel/klogger.h>
#include <kernel/tty.h>
#include <serial.h>

static char* levels[] = {
    "FINE",
    "DEBG",
    "INFO",
    " OK ",
    "WARN",
    "ERRO",
    "FERR",
    " ?? "
};

void log(int16_t level, const char* string)
{
    uint16_t index = level + 3;
    uint16_t color = vga_makeColor(VGA_GREY, terminal_getColor() >> 4);

    if (level == LOG_FINE)    color = vga_makeColor(VGA_DARK_GREY,   terminal_getColor() >> 4);
    if (level == LOG_DEBUG)   color = vga_makeColor(VGA_GREEN,       terminal_getColor() >> 4);
    if (level == LOG_INFO)    color = vga_makeColor(VGA_GREY,        terminal_getColor() >> 4);
    if (level == LOG_NORMAL)  color = vga_makeColor(VGA_LIGHT_GREEN, terminal_getColor() >> 4);
    if (level == LOG_WARNING) color = vga_makeColor(VGA_YELLOW,      terminal_getColor() >> 4);
    if (level == LOG_ERROR)   color = vga_makeColor(VGA_RED,         terminal_getColor() >> 4);
    if (level == LOG_FATAL)   color = vga_makeColor(VGA_LIGHT_RED,   terminal_getColor() >> 4);
    if (level >  LOG_FATAL || level < LOG_FINE)
    {
        color = vga_makeColor(VGA_LIGHT_BLUE, terminal_getColor() >> 4);
        index = LOG_FATAL-LOG_FINE+1;
    }

    terminal_putchar_Color('[', vga_makeColor(VGA_WHITE, terminal_getColor() >> 4));
    terminal_puts_Color(levels[index], color);
    terminal_puts_Color("] ", vga_makeColor(VGA_WHITE, terminal_getColor() >> 4));
    terminal_puts(string);

    serial_writechar(COM1, '[');
    serial_writes(COM1, levels[index]);
    serial_writes(COM1, "] ");
    serial_writes(COM1, string);
    io_wait();
}

void logFErr(const char* string)
{
    log(LOG_FATAL, string);
}

void logErro(const char* string)
{
    log(LOG_ERROR, string);
}

void logWarn(const char* string)
{
    log(LOG_WARNING, string);
}

void logNorm(const char* string)
{
    log(LOG_NORMAL, string);
}

void logInfo(const char* string) {
    log(LOG_INFO, string);
}

void logDebg(const char* string)
{
    log(LOG_DEBUG, string);
}

void logFine(const char* string)
{
    log(LOG_FINE, string);
}
