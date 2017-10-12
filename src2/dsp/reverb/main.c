
#include <string.h>
#include <stdlib.h>

#include "chip.h"
#include "reverb.h"

void M0APP_IRQHandler(void)
{
	Chip_CREG_ClearM0AppEvent();
}


uint32_t *i2s_val = (void *)0x1001fc00;


void main(void)
{
	NVIC_EnableIRQ(M0APP_IRQn);
	revmodel_t rev;

	rev_init(&rev, 24000);

	for(;;) {
		__WFI();

		float in = (*i2s_val) & 0xffff;
		in *= 3;
		float out1, out2;
		revmodel_process(&rev, in, &out1, &out2);

		uint32_t out = ((int)out1 << 16) | ((int)out2);
		Chip_I2S_Send(LPC_I2S0, out);
	}
}


/*
 * End
 */
