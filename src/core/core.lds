
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
	} > RAM3

	.text : { 
		_stext = .;
		*(.text*);
		_etext = .;
	} > RAM3

	.rodata : { 
		_srodata = .;
		*(.rodata*);
		_erodata = .;
	} > RAM3

	. = ALIGN(16);

	.data : {
		_sdata = .;
		*(.data*);
		_edata = .;
	} > RAM3

	.bss : {
		_sbss = .;
		*(.bss*);
		_ebss = .;
	} > RAM3
		
	_sstack = .;
}

/* Stack is at end of RAM3 with room for shared IPC block  */

PROVIDE(_estack = 0x2000f000);




