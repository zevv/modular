
OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_boot)

MEMORY {
	RAM1 (xrw) : ORIGIN = 0x10000000, LENGTH = 128K
	RAM2 (xrw) : ORIGIN = 0x10080000, LENGTH = 72K
	RAM3 (xrw) : ORIGIN = 0x20000000, LENGTH = 64K
}


SECTIONS {
	. = 0;

	_srom = .;

	.vectors : { 
		KEEP(*(.vectors)); 
	} > RAM2

	.text : { 
		_stext = .;
		*(.text*);
		_etext = .;
	} > RAM2

	.rodata : { 
		_srodata = .;
		*(.rodata*);
		_erodata = .;
	} > RAM2

	. = ALIGN(16);

	.data : {
		_sdata = .;
		*(.data*);
		_edata = .;
	} > RAM2

	.bss : {
		_sbss = .;
		*(.bss*);
		_ebss = .;
	} > RAM2
		
	_sstack = .;
}

/* Stack is at end of RAM2 with room for shared IPC block  */

PROVIDE(_estack = 0x10091800);




