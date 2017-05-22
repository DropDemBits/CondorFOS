.section .text
.extern isr_handler
.extern irq_handler
.extern trap_handler

.macro isr_no_errno int_num
.globl isr\int_num\()
isr\int_num\():
    push $0
    push $\int_num\()
    jmp prep_isr
.endm

.macro isr_errno int_num
.globl isr\int_num\()
isr\int_num\():
    
    push $\int_num\()
    jmp prep_isr    
.endm

.macro irq int_num
.globl isr\int_num\()
isr\int_num\():
    push $0
    push $\int_num\()
    jmp prep_isr
.endm

.macro trap int_num
.globl isr\int_num\()
isr\int_num\():
    push $0
    push $\int_num\()
    jmp prep_isr
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

prep_isr:
    # Because ESP and SS aren't pushed when there's no privilege change,
    # check if there's no privilege change to push ESP and SS.
    testl $3, 12(%esp) #CS
    jz change_stack

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
    addl $4, %esp
    
    popl %gs
    popl %fs
    popl %es
    popl %ds

    popa
    addl $8, %esp

    # Again, check for no privilege change to fix stack
    testl $3, 12(%esp)
    jz revert_change_stack
fini_isr:
    iret

# The following section was created using the help of source code
# from Sortix 

change_stack:
    # Moves stuff pushed by CPU up the stack by 8 bytes
    # Before:            After:          In Respect to previous ESP:
    #                    ---------------
    #                    | 0  INT NUM  | -8
    #                    ---------------
    #                    | 4 ERR CODE  | -4
    # ---------------    ---------------
    # | 0 INT NUM   |    | 8   EIP     | 0
    # ---------------    ---------------
    # | 4 ERR CODE  |    |12   CS      | 4
    # ---------------    ---------------
    # | 8   EIP     |    |16  EFLAGS   | 8
    # ---------------    ---------------
    # |12   CS      |    |20   ESP     | 12
    # ---------------    ---------------
    # |16  EFLAGS   |    |24   SS      | 16
    # ---------------    ---------------
    # |             |    |             |
    # ---------------    ---------------
    # |             |    |             |
    #       ...                ...     
    
    movl %eax, -4-8(%esp) # Used as temporary storage
    
    # Int num
    movl 0(%esp), %eax
    movl %eax, -8(%esp)
    
    # Err code
    movl 4(%esp), %eax
    movl %eax, -4(%esp)
    
    # EIP
    movl 8(%esp), %eax
    movl %eax, 0(%esp)
    
    # CS
    movl 12(%esp), %eax
    movl %eax, 4(%esp)
    
    # EFLAGS
    movl 16(%esp), %eax
    movl %eax, 8(%esp)
    
    # ESP
    movl %esp, %eax
    addl $(5*4), %eax
    movl %eax, 12(%esp)
    
    # SS
    movl %ss, %eax
    movl %eax, 16(%esp)
    
    # Finally, adjust ESP to new base
    subl $8, %esp
    # Also restore EAX
    movl -4(%esp), %eax
    
    jmp common_isr

revert_change_stack:
    # EAX: Scratch register
    # EBX: New ESP
    # ECX: New SS
    # If we are returning to the same stack, we need to be careful on where we
    # copy our things as to not overwrite anything. For that case, make it
    # so that we are effectively shifting the pushed data down 8 bytes.
    
    movl %eax, -4(%esp)
    movl %ebx, -8(%esp)
    movl %ecx, -12(%esp)
    
    # Setup registers
    movl 12(%esp), %ebx # New ESP
    movl 16(%esp), %ecx # New SS

    # Adjust new stack pointer for space for 3 dwords
    subl $(3*4), %ebx
    
    # Copy over saved data to new stack
    # Also, begin at 8 bytes after as to not overwrite data if switching to
    # the same stack
    
    # For these, it order doesn't matter as we aren't overwritting anything
    # important in the (same) stack
    
    # EAX
    movl -4(%esp), %eax
    movl %eax, -12(%ebx)

    # EBX
    movl -8(%esp), %eax
    movl %eax, -16(%ebx)
    
    # ECX
    movl -12(%esp), %eax
    movl %eax, -20(%ebx)
    
    # For these, we need to do EFLAGS first as to not be overwritten by EIP
    # If we are switching to the same stack
    
    # EFLAGS
    movl 8(%esp), %eax
    movl %eax, 8(%ebx)
    
    # CS
    movl 4(%esp), %eax
    movl %eax, 4(%ebx)
    
    # EIP
    movl 0(%esp), %eax
    movl %eax, 0(%ebx)
    
    # Switch over to new stack
    movw %cx, %ss
    movl %ebx, %esp
    
    # Finally, restore registers
    movl -12(%esp), %eax
    movl -16(%esp), %ebx
    movl -20(%esp), %ecx
    
    # Return to main routine
    jmp fini_isr