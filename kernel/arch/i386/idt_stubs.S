.section .text
.extern isr_handler
.extern irq_handler
.extern trap_handler

.macro isr_no_errno int_num
.globl isr\int_num\()
isr\int_num\():
    push $0
    push $\int_num\()
    jmp common_isr
.endm

.macro isr_errno int_num
.globl isr\int_num\()
isr\int_num\():
    
    push $\int_num\()
    jmp common_isr    
.endm

.macro irq int_num
.globl isr\int_num\()
isr\int_num\():
    push $0
    push $\int_num\()
    jmp common_irq
.endm

.macro trap int_num
.globl isr\int_num\()
isr\int_num\():
    push $0
    push $\int_num\()
    jmp common_trap
.endm

ISR_NO_ERRNO 0
ISR_NO_ERRNO 1
ISR_NO_ERRNO 2
ISR_NO_ERRNO 3
ISR_NO_ERRNO 4
ISR_NO_ERRNO 5
ISR_NO_ERRNO 6
ISR_NO_ERRNO 7
ISR_ERRNO 8
ISR_NO_ERRNO 9
ISR_ERRNO 10
ISR_ERRNO 11
ISR_ERRNO 12
ISR_ERRNO 13
ISR_ERRNO 14
ISR_NO_ERRNO 15
ISR_NO_ERRNO 16
ISR_ERRNO 17
ISR_NO_ERRNO 18
ISR_NO_ERRNO 19
ISR_NO_ERRNO 20
ISR_NO_ERRNO 21
ISR_NO_ERRNO 22
ISR_NO_ERRNO 23
ISR_NO_ERRNO 24
ISR_NO_ERRNO 25
ISR_NO_ERRNO 26
ISR_NO_ERRNO 27
ISR_NO_ERRNO 28
ISR_NO_ERRNO 29
ISR_ERRNO 30
ISR_NO_ERRNO 31
IRQ 32
IRQ 33
IRQ 34
IRQ 35
IRQ 36
IRQ 37
IRQ 38
IRQ 39
IRQ 40
IRQ 41
IRQ 42
IRQ 43
IRQ 44
IRQ 45
IRQ 46
IRQ 47

common_isr:
    pusha

    push %ds
    push %es
    push %fs
    push %gs
    
    movl $0x10, %edx
    movw %dx, %ds
    movw %dx, %es
    movw %dx, %fs
    movw %dx, %gs

    push %esp
    call isr_handler
    popl %eax
    
    popl %gs
    popl %fs
    popl %es
    popl %ds

    popa
    addl $8, %esp
    iret
common_irq:
    pusha

    push %ds
    push %es
    push %fs
    push %gs

    movw $0x10, %edx
    movw %dx, %ds
    movw %dx, %es
    movw %dx, %fs
    movw %dx, %gs

    push %esp
    call irq_handler
    popl %eax
    
    popl %gs
    popl %fs
    popl %es
    popl %ds

    popa
    addl $8, %esp
    iret
common_trap:
    pusha

    push %esp
    call trap_handler
    popl %eax

    popa
    addl $8, %esp
    iret
