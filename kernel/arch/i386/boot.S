/*
 * Copyright (C) 2017 DropDemBits <r3usrlnd@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
.align 4096 # Align to page boundary

# Initialize the stack
stack_top:
.globl stack_top
.skip 16384 # Reserve 16KiB
stack_bottom:
.globl stack_bottom

tss_stack_top:
.skip 4096
tss_stack_bottom:

df_stack_bottom:
.skip 4096
df_stack_top:


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
tss_descriptor:
    #TSS
    .byte 0x67, 0x00, 0x00, 0x00, 0x00, 0x89, 0x80, 0xC0
gdt_end:

.globl idt_data
idt_data:
	.skip 16384
idt_end:

idtr:
	.word idt_end - idt_data - 1
	.long idt_data

.align 4096

# TSS Must be aligned to page boundary
.global tss_start
tss_start:
	.long 0
	.long tss_stack_top
	.long 0x10
	.skip 0x5C # Size of tss
tss_end:

.section .rodata

# Main code
.section .text
.extern idt_init
.global _start
.type _start, @function
_start:
	cli

	# Initialize stack
	movl $(stack_top - KERNEL_VIRTUAL_BASE), %esp

    # Preserve EAX and EBX
	push %ebx
	push %eax

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
	# Restore EAX & EBX
	popl %eax
	popl %ebx

    # Initialize paging

    # Setup registers
    xor %esi, %esi
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

    lea .LtoHigher, %edx
    jmp *%edx

.LtoHigher:

    # Disable identity paging
    movl $0, pd_start + 0x000
    movl $0, pd_start + 0x004

    #Flush TLB
    movl %cr3, %edx
    movl %edx, %cr3

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

	# Set stack
	movl $stack_top, %esp
	push %eax
	push %ebx

	push $idt_data
	call idt_init
	addl $4, %esp

	lidt (idtr)

    # Setup TSS
    movl $tss_descriptor, %edi
    movl $tss_start, %edx

    movw %dx, 2(%edi)
    shr $16, %edx
    movb %dl, 4(%edi)
    movb %dh, 7(%edi)

    movl $tss_start, %edi
    movw $0x10, 8(%edi)
    movl %esp, 4(%edi)
    movw $0x68, 102(%edi)

    # Load TSS
    movw $(5 << 3), %dx
    ltr %dx

    # Restore EBX
	popl %ebx
	popl %eax
    orl $KERNEL_VIRTUAL_BASE, %ebx

	# Before anything interactive, init kernel
	# Also push multiboot stuff (EAX is still on stack)
	push %eax
	push %ebx # Address to info structure
	call kinit

	# kmain is launched as a seperate process

	# Hang if there's nothing to do
.hang:
    cli
	hlt
	jmp .hang

.size _start, . - _start
