
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "chip.h"
#include "osc.h"

void M0APP_IRQHandler(void)
{
	Chip_CREG_ClearM0AppEvent();
}


uint32_t *i2s_val = (void *)0x1001fc00;

	
static struct osc osc1, osc2;

union sample {
	uint32_t u32;
	int16_t s16[2];
};

void main(void)
{
	LPC_GPIO_PORT->B[1][11] = 0;

	NVIC_EnableIRQ(M0APP_IRQn);

	osc_init(&osc1, 48000);
	osc_init(&osc2, 48000);
	osc_set_freq(&osc1, 400);
	osc_set_freq(&osc2, 403);
	int n = 0;
	float gain = 0.00001;

	for(;;) {
		__WFI();
	
		LPC_GPIO_PORT->B[1][11] = (n++ & 0xf00) != 0;

		float out1 = osc_gen_linear(&osc1) * 0.01;
		float out2 = osc_gen_linear(&osc2) * 0.01;
		
		union sample s;

		s.s16[0] = out1 * 16000 * gain;
		s.s16[1] = out2 * 16000 * gain;

		if(Chip_I2S_GetTxLevel(LPC_I2S0) < 4) {
			Chip_I2S_Send(LPC_I2S0, s.u32);
		}

		if(gain < 1.0) gain += 0.00005;
	}
}


/*
 * End
 */
