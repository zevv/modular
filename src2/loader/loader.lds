
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
		*(.data*);
		_edata = .;
	} > RAM1

	.bss : {
		_sbss = .;
		*(.bss*);
		_ebss = .;
	} > RAM1
		
	_sstack = .;
}

/* Stack is at end of RAM1 */

PROVIDE(_estack = 0x10020000);




