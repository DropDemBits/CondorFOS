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

.section .bss
# Initialize the stack
stack_bottom:
.skip 16384 # Reserve 16KiB
stack_top:

.section .data
# GDT sections
gdt_data:
	# Null descriptor
	.long 0, 0
	# Kmode code segment
	.byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00
	# Kmode data segment
	.byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00
	# Usermode code segment
	.byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFE, 0xCF, 0x00
	# Usermode data segment
	.byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xF2, 0xCF, 0x00
gdt_end:

gdtr:
	.word gdt_end - gdt_data - 1
	.long gdt_data
.globl idt_data
idt_data:
	.skip 16384
idt_end:

idtr:
	.word idt_end - idt_data - 1
	.long idt_data

# Main code
.section .text
.extern idt_init
.global _start
.type _start, @function
_start:
	cli

	# Initialize stack
	movl $stack_top, %esp
	# Preserve EAX and EBX
	movl %eax, %esi
	movl %ebx, %edi

	# Load GDT
	lgdt (gdtr)

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

	# Before anything interactive, init kernel
	# Also push grub stuff
	push %esi # Multiboot magic thing
	push %edi # Address to info structure
	call kinit

	# Go into the main kernel
	call kmain

	# Hang if there's nothing to do
	cli
.hang:
	hlt
	jmp .hang
.size _start, . - _start
