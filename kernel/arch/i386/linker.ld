/* Kernel entry point */
ENTRY(_start)

SECTIONS
{
	/* Load the kernel at 1M */
	. = 1M;

	/* Put the multiboot header and code sections at the very beginning of the file */
	.text BLOCK(4K) : ALIGN(4K) {
		*(.multiboot)
		*(.text)
	}

	/* Read-Only data */
	.rodata BLOCK(4K) : ALIGN(4K) {
		*(.rodata)
	}

	/* Read-Write data */
	.data BLOCK(4K) : ALIGN(4K) {
		*(.data)
	}

	/* Bootstrap stack / Uninitialized data */
	.bss BLOCK(4K) : ALIGN(4K) {
		*(.COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}
        kernel_end = .;

}
