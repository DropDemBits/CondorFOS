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

# Main code
.section .text
.global _start
.type _start, @function
_start:
	cli
	# Initialized stack
	movl $stack_top, %esp
	
	# Hang if there's nothing to do
	cli
.hang:
	hlt
	jmp .hang
.size _start, . - _start
