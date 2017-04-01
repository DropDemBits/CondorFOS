#include <kernel/pic.h>
#include <io.h>

#define PIC1         0x20 /* IOBase addr for PIC1 */
#define PIC2         0xA0 /* IOBase addr for PIC2 */
#define PIC1_COMMAND PIC1
#define PIC2_COMMAND PIC2
#define PIC1_DATA    PIC1_COMMAND+1
#define PIC2_DATA    PIC2_COMMAND+1
/* Data codes */
#define PIC_EOI      0x20

/* Control word bits*/
#define ICW1_ICW4	    0x01	/* ICW4 (not) needed */
#define ICW1_SINGLE  	0x02	/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04    /* Call address interval 4 (8) */
#define ICW1_LEVEL	    0x08	/* Level triggered (edge) mode */
#define ICW1_INIT	    0x10	/* Initialization - required! */

#define ICW4_8086	    0x01	/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	    0x02	/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08	/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C	/* Buffered mode/master */
#define ICW4_SFNM	    0x10	/* Special fully nested */

void pic_init(uint16_t irq0Base, uint16_t irq8Base)
{
    uint8_t mask1, mask2;
    mask1 = inb(PIC1_DATA);
    mask2 = inb(PIC2_DATA);

    //1st Control word
    outb(PIC1_COMMAND, ICW1_ICW4 | ICW1_INIT);
    io_wait();
    outb(PIC2_COMMAND, ICW1_ICW4 | ICW1_INIT);
    io_wait();

    //Offsets
    outb(PIC1_DATA, irq0Base);
    io_wait();
    outb(PIC2_DATA, irq8Base);
    io_wait();

    //IRQ Maps
    //Master: Slave PIC at service line 2
    outb(PIC1_DATA, 0b00000100);
    io_wait();
    //Slave: Master at ID1
    outb(PIC2_DATA, 0b00000010);
    io_wait();

    //ICW4: 8086 mode
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    //Reload masks
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

void pic_ack(uint16_t irq)
{
    if(irq > 15) return;
    //Also notify Slave PIC if irq >= IRQ8
    if(irq >= 8)
        outb(PIC2_COMMAND, PIC_EOI);

    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_maskIRQ(uint16_t irq)
{
    if(irq > 15) return;
    uint16_t port = PIC1_DATA;
    if(irq > 7) {
        irq -= 8;
        port = PIC2_DATA;
    }
    uint8_t mask = inb(port) | (1 << irq);
    outb(port, mask);
}

void pic_unmaskIRQ(uint16_t irq)
{
    if(irq > 15) return;
    uint16_t port = PIC1_DATA;
    if(irq > 7) {
        irq -= 8;
        port = PIC2_DATA;
    }
    uint8_t mask = inb(port) & ~(1 << irq);
    outb(port, mask);
}

/* Reg is ioaddr */
uint32_t pic_read(uint32_t reg)
{
    if(reg != PIC1_COMMAND && reg != PIC2_COMMAND && reg != PIC1_DATA && reg != PIC2_DATA) return 0;
    return inb(reg);
}

void pic_write(uint32_t reg, uint32_t value)
{
    if(reg != PIC1_COMMAND && reg != PIC2_COMMAND && reg != PIC1_DATA && reg != PIC2_DATA) return;
    outb(reg, value);
    return;
}
