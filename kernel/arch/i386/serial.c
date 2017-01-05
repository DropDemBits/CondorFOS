#include <string.h>
#include <serial.h>

void serial_init(uint16_t port, uint16_t divisor)
{
    //Disable interrupt comm
    outb(port + 1, 0x00);
    //Set DLAB
    outb(port + 3, 0x80);
    //Set Divisor
    if(divisor == 0) divisor = 0x0001;
    outb(port + 0, divisor & 0xFF);
    outb(port + 1, divisor >> 8);

    //Unset DLAB, Do 8N1 (8 bits, no pairity, 1 stop bit)
    outb(port + 3, 0x03);
    //FCR (Enable & clear buffers, Threshold to 14 bytes)
    outb(port + 2, 0xC7);
    //MCR (RTS, DTR, Disable irqs)
    outb(port + 4, 0x03);
}

uint8_t serial_readReady(uint16_t port)
{
    return inb(port+5) & 1;
}

uint8_t serial_writeReady(uint16_t port)
{
    return inb(port+5) & 0x20;
}

char serial_readchar(uint16_t port)
{
    while(serial_readReady(port) == 0);

    return inb(port);
}

char* serial_reads(uint16_t port, uint16_t length)
{
    while(serial_readReady(port) == 0);
    char* store = (char*) 0x500;

    for(uint16_t i = 0; i < length; i++) store[i] = inb(port);
    return store;
}

void serial_writechar(uint16_t port, char uc)
{
    while(serial_writeReady(port) == 0);

    outb(port, uc);
}

void serial_writes(uint16_t port, char* string)
{
    while(serial_writeReady(port) == 0);

    for(uint16_t i = 0; i < strlen(string); i++) outb(port, string[i]);
}
