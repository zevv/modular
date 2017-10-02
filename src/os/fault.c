#include "os/printd.h"
#include "os/board.h"

void HardFault_Handler(void)
{
	extern uint8_t _estack;
	extern uint8_t _stext;
	extern uint8_t _etext;

        LPC_GPIO_PORT->DIR[1] = 1UL << 11; 

	volatile int i;
	printd("\n\n** HARD FAULT**\n");

	uint32_t j;
	uint32_t *sp = &j;

	printd("sp: %08x\n\n", sp);


	uint32_t *p = sp;
	while(p < (uint32_t *)&_estack) {
		if(*p >= (uint32_t)&_stext && *p < (uint32_t)&_etext) {
			printd("%08x\n", *p);
		}
		p++;
	}

	printd("\n");

	for(j=0; j<32; j++) {
		printd("%08x ", *(sp+j));
		if((j % 8) == 7) printd("\n");
	}

	for(;;) {
		LPC_GPIO_PORT->B[1][11] = 0;
		for(i=0; i<100000; i++);
		LPC_GPIO_PORT->B[1][11] = 1;
		for(i=0; i<2000000; i++);
	}
	for(;;);
}
