#include <string.h>
#include <serial.h>

void serial_init(uint16_t port, uint16_t divisor)
{
    //Disable interrupt comm
    outb(port + 1, 0x00);
    io_wait();
    //Set DLAB
    outb(port + 3, 0x80);
    io_wait();
    //Set Divisor
    if(divisor == 0) divisor = 0x0001;
    outb(port + 0, divisor & 0xFF);
    io_wait();
    outb(port + 1, divisor >> 8);
    io_wait();
    
    //Unset DLAB, Do 8N1 (8 bits, no pairity, 1 stop bit)
    outb(port + 3, 0x03);
    io_wait();
    //FCR (Enable & clear buffers, Threshold to 14 bytes)
    outb(port + 2, 0xC7);
    io_wait();
    //MCR (RTS, DTR, Disable irqs)
    outb(port + 4, 0x03);
    io_wait();
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

char* serial_reads(uint16_t port, uint16_t length, char* dest)
{
    while(serial_readReady(port) == 0);

    for(uint16_t i = 0; i < length; i++) dest[i] = inb(port);
    return dest;
}

void serial_writechar(uint16_t port, char uc)
{
    while(serial_writeReady(port) == 0);
    
    if(uc == '\n') {
        outb(port, '\r');
        outb(port, '\n');
    }
    else outb(port, uc);
}

void serial_writes(uint16_t port, const char* string)
{
    while(serial_writeReady(port) == 0);

    for(uint16_t i = 0; i < strlen(string); i++)
    {
        io_wait();
        if(string[i] == '\n')
        {
            outb(port, '\r');
            outb(port, '\n');
        }
        else
            outb(port, string[i]);
    }
    while(serial_writeReady(port) == 0);
}
