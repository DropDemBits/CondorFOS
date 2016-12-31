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
