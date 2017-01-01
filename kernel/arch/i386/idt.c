#include <idt.h>
#include <kernel/klogger.h>
#include <kernel/pic_hal.h>
#include <condor.h>

// ISRs
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

static uint32_t isrs[256];
static char* predefMSGS[] = {
    "Division by 0",
    "Debug",
    "NMI",
    "Breakpoint",
    "Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "Device not available",
    "Double Fault",
    "CSegment Overrun",
    "Invalid TSS",
    "Invalid Segment",
    "Stack Segment Fault",
    "GP Fault",
    "Page Fault",
    "Intel Reserved",
    "x87 Floating Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating Point Exception",
    "Virtual Error",
    "Intel Reserved",
    "Intel Reserved",
    "Intel Reserved",
    "Intel Reserved",
    "Intel Reserved",
    "Intel Reserved",
    "Intel Reserved",
    "Intel Reserved",
    "Intel Reserved",
    "Security Exception",
    "Intel Reserved",
};
static uint32_t* descriptors;

void isr_handler(struct CPUStack stack_cpu, struct Pusha stack_pusha)
{
    uint16_t int_num = 0;
    uint16_t err_code = 0;

    asm("movl %%esp, %%esi\n\t"
        "addl $36, %%esi\n\t"
        "movl (%%esi), %0\n\t"
        "addl $4, %%esi\n\t"
        "movl (%%esi), %1\n\t"
        :"=ra"(int_num), "=rd"(err_code));

    if(!isrs[int_num]) {
        if(int_num > 31) int_num = 31;
        kpanic(predefMSGS[int_num]);
    } else {
        void (*handler)(struct CPUStack, struct Pusha);
        if(isrs[int_num]) {
            handler = (void*) isrs[int_num];
            handler(stack_cpu, stack_pusha);
        }
    }
}

void irq_handler(struct CPUStack stack_cpu)
{
    uint16_t int_num = 0;
    uint16_t err_code = 0;

    asm("movl %%esp, %%esi\n\t"
        "addl $40, %%esi\n\t"
        "movl (%%esi), %0\n\t"
        "addl $4, %%esi\n\t"
        "movl (%%esi), %1\n\t"
        :"=ra"(int_num), "=rd"(err_code));

    void (*handler)(struct CPUStack);
    if(isrs[int_num]) {
        handler = (void*) isrs[int_num];
        handler(stack_cpu);
    }
    ic_ack(int_num-32);
}

void trap_handler(struct CPUStack stack_cpu)
{
    uint16_t int_num = 0;
    uint16_t err_code = 0;

    asm("movl %%esp, %%esi\n\t"
        "addl $36, %%esi\n\t"
        "movl (%%esi), %0\n\t"
        "addl $4, %%esi\n\t"
        "movl (%%esi), %1\n\t"
        :"=ra"(int_num), "=rd"(err_code));

    void (*handler)(struct CPUStack);
    if(isrs[int_num]) {
        handler = (void*) isrs[int_num];
        handler(stack_cpu);
    }
}

static void idt_registerInterrupt(uint16_t int_num, uint32_t func_addr, uint16_t gdt_selector, uint8_t type_attrib)
{
    if(int_num > 255) return;
    int_num *= 2;
    uint32_t desc_lower =(func_addr & 0xFFFF) | (gdt_selector << 16);
    uint32_t desc_higher = 0x00 | (type_attrib << 8) | (func_addr & 0xFFFF0000);
    descriptors[int_num+0] = desc_lower;
    descriptors[int_num+1] = desc_higher;
}

void idt_addISR(uint16_t int_num, uint32_t addr)
{
    isrs[int_num] = addr;
}

void idt_clearISR(uint16_t int_num)
{
    isrs[int_num] = 0x0;
}

void idt_init(uint32_t memory_location)
{
    descriptors = (uint32_t*)memory_location;

    idt_registerInterrupt( 0, (uint32_t)isr0,  0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt( 1, (uint32_t)isr1,  0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt( 2, (uint32_t)isr2,  0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt( 3, (uint32_t)isr3,  0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt( 4, (uint32_t)isr4,  0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt( 5, (uint32_t)isr5,  0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt( 6, (uint32_t)isr6,  0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt( 7, (uint32_t)isr7,  0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt( 8, (uint32_t)isr8,  0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt( 9, (uint32_t)isr9,  0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(10, (uint32_t)isr10, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(11, (uint32_t)isr11, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(12, (uint32_t)isr12, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(13, (uint32_t)isr13, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(14, (uint32_t)isr14, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(15, (uint32_t)isr15, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(16, (uint32_t)isr16, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(17, (uint32_t)isr17, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(18, (uint32_t)isr18, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(19, (uint32_t)isr19, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(20, (uint32_t)isr20, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(21, (uint32_t)isr21, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(22, (uint32_t)isr22, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(23, (uint32_t)isr23, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(24, (uint32_t)isr24, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(25, (uint32_t)isr25, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(26, (uint32_t)isr26, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(27, (uint32_t)isr27, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(28, (uint32_t)isr28, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(29, (uint32_t)isr29, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(30, (uint32_t)isr30, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(31, (uint32_t)isr31, 0x08, ISR_32_INTRGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    ic_init(32);
    //Hardware intrs
    idt_registerInterrupt(32, (uint32_t)irq0,  0x08, ISR_32_TRAPGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(33, (uint32_t)irq1,  0x08, ISR_32_TRAPGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(34, (uint32_t)irq2,  0x08, ISR_32_TRAPGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(35, (uint32_t)irq3,  0x08, ISR_32_TRAPGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(36, (uint32_t)irq4,  0x08, ISR_32_TRAPGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(37, (uint32_t)irq5,  0x08, ISR_32_TRAPGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(38, (uint32_t)irq6,  0x08, ISR_32_TRAPGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(39, (uint32_t)irq7,  0x08, ISR_32_TRAPGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(40, (uint32_t)irq8,  0x08, ISR_32_TRAPGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(41, (uint32_t)irq9,  0x08, ISR_32_TRAPGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(42, (uint32_t)irq10, 0x08, ISR_32_TRAPGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(43, (uint32_t)irq11, 0x08, ISR_32_TRAPGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(44, (uint32_t)irq12, 0x08, ISR_32_TRAPGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(45, (uint32_t)irq13, 0x08, ISR_32_TRAPGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(46, (uint32_t)irq14, 0x08, ISR_32_TRAPGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    idt_registerInterrupt(47, (uint32_t)irq15, 0x08, ISR_32_TRAPGATE | ISR_ATR_PRESENT | ISR_ATR_RING0);
    //Mask all irqs but the pit irq
    //ic_maskIRQ(0);
    //ic_maskIRQ(1);
    ic_maskIRQ(2);
    ic_maskIRQ(3);
    ic_maskIRQ(4);
    ic_maskIRQ(5);
    ic_maskIRQ(6);
    ic_maskIRQ(7);
    ic_maskIRQ(8);
    ic_maskIRQ(9);
    ic_maskIRQ(10);
    ic_maskIRQ(11);
    ic_maskIRQ(12);
    ic_maskIRQ(13);
    ic_maskIRQ(14);
    ic_maskIRQ(15);
}
