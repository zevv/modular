#include <ctype.h>

#include "board.h"

#include "cdc_uart.h"
#include "chip.h"
#include "printd.h"
#include "uart.h"
#include "cmd.h"

extern uint8_t _sheap;
extern uint8_t _eheap;

static uint8_t *sbrk_end = &_sheap;



static int on_mem(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	static uint32_t addr = 0;

	if(argc == 0) return 0;

	char cmd = argv[0][0];

	if(cmd == 'd') {

		if(argc == 2) {
			addr = strtoll(argv[1], NULL, 0);
			if(argv[1][0] == 's') addr = 0x1001ff80;
		}

		cmd_hexdump(cli, 0, 128, addr);

		addr += 128;
		return 1;
	}

	if(cmd == 'i') {
		extern uint8_t _stext, _etext;
		extern uint8_t _sdata, _edata;
		extern uint8_t _srodata, _erodata;
		extern uint8_t _sbss, _ebss;
		extern uint8_t _sheap;
		extern uint8_t _sstack, _estack;
		
		cmd_printd(cli, "text   0x%08x-0x%08x: %3d Kb\n", &_stext,   &_etext,  (&_etext  - &_stext)  / 1024);
		cmd_printd(cli, "rodata 0x%08x-0x%08x: %3d Kb\n", &_srodata, &_erodata,(&_erodata- &_srodata)/ 1024);
		cmd_printd(cli, "data   0x%08x-0x%08x: %3d Kb\n", &_sdata,   &_edata,  (&_edata  - &_sdata)  / 1024);
		cmd_printd(cli, "bss    0x%08x-0x%08x: %3d Kb\n", &_sbss,    &_ebss,   (&_ebss   - &_sbss)   / 1024);
		cmd_printd(cli, "heap   0x%08x-0x%08x: %3d Kb\n", &_sheap,   &_eheap,  (&_eheap  - &_sheap)  / 1024);
		cmd_printd(cli, "sbrk   0x%08x-0x%08x: %3d Kb\n", &_sheap,   sbrk_end, (sbrk_end - &_sheap)  / 1024);
		cmd_printd(cli, "stack  0x%08x-0x%08x: %3d Kb\n", &_sstack,  &_estack, (&_estack - &_sstack) / 1024);

		return 1;
	}

	if(cmd == 't') {
		size_t s = 0;
		for(;;) {
			void * p = malloc(1024);
			if(p) {
				s ++;
			} else {
				cmd_printd(cli, "%d Kb allocated\n", s);
				for(;;);
			}
		}
	}

	return 0;
}


CMD_REGISTER(mem, on_mem, "d[ump] [ADDR] | i[nfo]");


/*
 * End
 */

