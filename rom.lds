OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(ResetISR)

MEMORY {
	RAM1 (xrw) : ORIGIN = 0x10000000, LENGTH = 128K
	RAM2 (xrw) : ORIGIN = 0x10080000, LENGTH = 72K
	RAM3 (xrw) : ORIGIN = 0x20000000, LENGTH = 64K
}


SECTIONS {
	. = 0x10000000;

	_srom = .;

	.vectors : { 
		KEEP(*(.isr_vector)); 
		KEEP(*(.after_vectors)); 
	} > RAM1

	.text : { 
		_stext = .;
		*(.text*);
		_etext = .;
	} > RAM1

	.rodata : { 
		_srodata = .;
		*(.rodata*);
		_erodata = .;
	} > RAM1

	. = ALIGN(16);

	.data : {
		_sdata = .;
		__data_section_table = .;
		*(.data*);
		_edata = .;
		__data_section_table_end = .;
	} > RAM1

	.bss : {
		_sbss = .;
		__bss_section_table = .;
		*(.bss*);
		_ebss = .;
		__bss_section_table_end = .;
	} > RAM1
	
	
}

PROVIDE(_sheap  = 0x10080000);
PROVIDE(_eheap  = 0x10092000);

PROVIDE(_sstack = 0x1001e000);
PROVIDE(_estack = 0x10020000);

PROVIDE(_vStackTop = 0x10020000);

