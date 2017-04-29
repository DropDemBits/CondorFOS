/* Kernel entry point */
ENTRY(_start)

SECTIONS
{
	/* Load the kernel at 0xC0100000 */
	. = 0xC0100000;
        
    kernel_start = .;
	
    /* Put the multiboot header and code sections at the very beginning of the file */
	.text ALIGN(4K) : AT(ADDR(.text) - 0xC0000000) {
		*(.multiboot)
		*(.text)
	}

	/* Read-Only data */
	.rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xC0000000) {
		*(.rodata)
	}
        
    ro_end = .;

	/* Read-Write data */
	.data ALIGN(4K) : AT(ADDR(.data) - 0xC0000000) {
		*(.data)
	}

	/* Bootstrap stack / Uninitialized data */
	.bss ALIGN(4K) : AT(ADDR(.bss) - 0xC0000000) {
		*(.COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}
    
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}

kernel_end = .;
