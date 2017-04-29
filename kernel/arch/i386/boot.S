# Multiboot header
.set FLAGS,    0x3
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

# Mulitboot header structure
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Kernel Virtual Memory base
.globl KERNEL_VIRTUAL_BASE
.set KERNEL_VIRTUAL_BASE, 0xC0000000

.section .bss
# Initialize the stack
stack_bottom:
.globl stack_bottom
.skip 16384 # Reserve 16KiB
stack_top:
.globl stack_top

.align 4096 # Align to page boundary

pd_start:
.skip 4096
pd_end:

pt0_start:
.skip 4096
pt0_end:

pt1_start:
.skip 4096
pt1_end:

pt2_start:
.skip 4096
pt2_end:

# TSS Must be aligned to page boundary
tss_start:
.skip 0x68 # Size of tss
tss_end:

.global sse2_support
sse2_support:
.skip 1

.section .data
# GDT sections
gdt_data:
	# Null descriptor
	.word gdt_end - gdt_data - 1
	.long gdt_data
	.word 0
	# Kmode code segment
	.byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00
	# Kmode data segment
	.byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00
	# Usermode code segment
	.byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFA, 0xCF, 0x00
	# Usermode data segment
	.byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xF2, 0xCF, 0x00
    #TSS
    .byte 0x67, 0x00
    .word 0x0000
    .byte 0x00, 0x89, 0x80, 0xC0
gdt_end:

.globl idt_data
idt_data:
	.skip 16384
idt_end:

idtr:
	.word idt_end - idt_data - 1
	.long idt_data

.align 4096

.section .rodata

# Main code
.section .text
.extern idt_init
.global _start
.type _start, @function
_start:
	cli
        
    # Preserve EAX and EBX
	movl %ebx, 0x00000500
	movl %eax, 0x00000504
    
    # Check for SSE & SSE2
    movl $0x1, %eax
    cpuid
    
    test $(1<<26), %edx
    jz .no_sse
    
    # Go Ahead and setup SSE
    movl %cr0, %eax
    andl $(~4), %eax
    orl $2, %eax
    movl %eax, %cr0

    movl %cr4, %eax
    orl $0x600, %eax
    movl %eax, %cr4
    
    movb $0x1, (sse2_support - KERNEL_VIRTUAL_BASE)
    
.no_sse:
    # Initialize paging
    
    # Setup registers
    movl $0, %esi
    movl $(pt0_start - KERNEL_VIRTUAL_BASE), %edi
    movl $2048, %ecx

.Lbegin_paging:
    movl %esi, %edx
    
    cmpl $(kernel_end - KERNEL_VIRTUAL_BASE + 0x1000), %esi
    jge .Ldone
    cmpl $(kernel_start - KERNEL_VIRTUAL_BASE), %esi
    jl .Lrw
    cmpl $(ro_end - KERNEL_VIRTUAL_BASE), %esi
    jge .Lrw

.Lro:
    orl $0x1, %edx
    jmp .Lprep
    
.Lrw:
    orl $0x3, %edx
.Lprep: 
    movl %edx, (%edi)
    # Prepare for next cycle
    addl $0x4, %edi
    addl $0x1000, %esi
    loop .Lbegin_paging

.Ldone:
    # Setup pages tables to be at index 0 and 768 (so that all kaddrs are above 0xC00000000)
    movl $(pt0_start - KERNEL_VIRTUAL_BASE + 0x3), pd_start - KERNEL_VIRTUAL_BASE + 0x000
    movl $(pt1_start - KERNEL_VIRTUAL_BASE + 0x3), pd_start - KERNEL_VIRTUAL_BASE + 0x004
    movl $(pt0_start - KERNEL_VIRTUAL_BASE + 0x3), pd_start - KERNEL_VIRTUAL_BASE + 768*4
    movl $(pt1_start - KERNEL_VIRTUAL_BASE + 0x3), pd_start - KERNEL_VIRTUAL_BASE + 769*4
    movl $(pd_start - KERNEL_VIRTUAL_BASE + 0x3), pd_start - KERNEL_VIRTUAL_BASE + 0xffc
    
    # Load CR3
    movl $(pd_start - KERNEL_VIRTUAL_BASE), %edi
    movl %edi, %cr3
        
    # Enable all goodies (pse, pge)
    movl %cr4, %edx
    orl $0x90, %edx
    movl %edx, %cr4

    # Enable paging and WP
    movl %cr0, %edx
    orl $0x80010000, %edx
    movl %edx, %cr0
        
    lea .LtoHigher, %eax
    jmp *%eax
        
.LtoHigher:

    # Disable identity paging
    movl $0, pd_start + 0x000
    movl $0, pd_start + 0x004

    #Flush TLB
    movl %cr3, %edx
    movl %edx, %cr3

	# Initialize stack
	movl $stack_top, %esp

	# Load GDT
	lgdt (gdt_data)

	# Flush segment selectors
	movw $0x10, %dx
	movw %dx, %ds
	movw %dx, %es
	movw %dx, %fs
	movw %dx, %gs
	movw %dx, %ss
	jmp $8, $.flush_cs
.flush_cs:

	push $idt_data
	call idt_init

	lidt (idtr)
        
    # Restore EAX and EBX
    movl 0xC0000500, %ebx
	movl 0xC0000504, %eax
        
    orl $KERNEL_VIRTUAL_BASE, %ebx
        
	# Before anything interactive, init kernel
	# Also push multiboot stuff
	push %eax # Multiboot magic thing
	push %ebx # Address to info structure
	call kinit

	# Go into the main kernel
	call kmain

	# Hang if there's nothing to do
.hang:
        cli
	hlt
	jmp .hang
        
.size _start, . - _start
