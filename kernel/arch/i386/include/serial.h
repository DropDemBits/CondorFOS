#include <stddef.h>
#include <io.h>

#ifndef SERIAL_H
#define SERIAL_H

#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8

void serial_init(uint16_t port, uint16_t divisor);
uint8_t serial_readReady(uint16_t port);
uint8_t serial_writeReady(uint16_t port);
char serial_readchar(uint16_t port);
char* serial_reads(uint16_t port, uint16_t length);
void serial_writechar(uint16_t port, char uc);
void serial_writes(uint16_t port, char* string, uint16_t length);

#endif
