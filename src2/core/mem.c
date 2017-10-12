#include <ctype.h>

#include "chip.h"
#include "printd.h"
#include "cmd.h"


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

		cmd_hexdump(cli, (void *)addr, 128, addr);

		addr += 128;
		return 1;
	}

	if(cmd == 'i') {
		extern uint8_t _stext, _etext;
		extern uint8_t _sdata, _edata;
		extern uint8_t _srodata, _erodata;
		extern uint8_t _sbss, _ebss;
		extern uint8_t _sstack, _estack;
		
		cmd_printd(cli, "text   0x%08x-0x%08x: %3d Kb\n", &_stext,   &_etext,  (&_etext  - &_stext)  / 1024);
		cmd_printd(cli, "rodata 0x%08x-0x%08x: %3d Kb\n", &_srodata, &_erodata,(&_erodata- &_srodata)/ 1024);
		cmd_printd(cli, "data   0x%08x-0x%08x: %3d Kb\n", &_sdata,   &_edata,  (&_edata  - &_sdata)  / 1024);
		cmd_printd(cli, "bss    0x%08x-0x%08x: %3d Kb\n", &_sbss,    &_ebss,   (&_ebss   - &_sbss)   / 1024);
		cmd_printd(cli, "stack  0x%08x-0x%08x: %3d Kb\n", &_sstack,  &_estack, (&_estack - &_sstack) / 1024);

		return 1;
	}

	return 0;
}


CMD_REGISTER(mem, on_mem, "d[ump] [ADDR] | i[nfo]");


/*
 * End
 */

