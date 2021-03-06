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

#include <stddef.h>
#include <io.h>

#ifndef SERIAL_H
#define SERIAL_H

/* Serial Ports */
#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8

/**
 * Initializes a serial port
 * @param port The port to initialize
 * @param divisor The divisor of the baud rate
 */
void serial_init(uint16_t port, uint16_t divisor);

/**
 * Checks if the port is ready for reading
 * @param port The port to check the status
 * @return 1 if ready, 0 otherwise
 */
uint8_t serial_readReady(uint16_t port);

/**
 * Checks if the port is ready for writing
 * @param port The port to check the status
 * @return 1 if ready, 0 otherwise
 */
uint8_t serial_writeReady(uint16_t port);

/**
 * Reads a char from the port
 * @param port The port to read from
 * @return A char from the port
 */
char serial_readchar(uint16_t port);

/**
 * Reads a series of chars from the port
 * @param port The port to read from
 * @param length The amount of chars to read
 * @param dest The destination for the data
 * @return A pointer to the data (usually dest)
 */
char* serial_reads(uint16_t port, uint16_t length, char* dest);

/**
 * Writes a char to the port
 * @param port The port to write to
 * @param uc The char to write
 */
void serial_writechar(uint16_t port, const char uc);

/**
 * Writes a series of chars to the port
 * @param port The port to write to
 * @param string The series of chars to write
 */
void serial_writes(uint16_t port, const char* string);

#endif
