
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "chip.h"
#include "osc.h"
#include "reverb.h"

void M0APP_IRQHandler(void)
{
	Chip_CREG_ClearM0AppEvent();
}


uint32_t *i2s_val = (void *)0x1001fc00;

	
static revmodel_t rev;
static struct osc osc1, osc2;

void main(void)
{
	LPC_GPIO_PORT->B[1][11] = 0;

	NVIC_EnableIRQ(M0APP_IRQn);

	rev_init(&rev, 24000);
	osc_init(&osc1, 48000);
	osc_init(&osc2, 48000);
	osc_set_freq(&osc1, 400);
	osc_set_freq(&osc2, 403);
	int n = 0;
	float gain = 0.00001;

	for(;;) {
		__WFI();
	
		LPC_GPIO_PORT->B[1][11] = (n++ & 0xf00) != 0;

		int16_t in1 = *i2s_val & 0xffff;
		float in = in1 / 65535.0;

		float out1, out2;
		revmodel_process(&rev, in, &out1, &out2);
		
		out1 += osc_gen_linear(&osc1) * 0.01;
		out2 += osc_gen_linear(&osc2) * 0.01;

		out1 = sqrtf(out1);
		out2 = sqrtf(out2);

		out1 = (out1 < -1.0) ? -1.0 : out1;
		out2 = (out2 < -1.0) ? -1.0 : out2;
		out1 = (out1 > +1.0) ? +1.0 : out1;
		out2 = (out2 > +1.0) ? +1.0 : out2;

		int iout1 = out1 * 16000 * gain;
		int iout2 = out2 * 16000 * gain;

		uint32_t out = (iout1 << 16) | iout2;

		if(Chip_I2S_GetTxLevel(LPC_I2S0) < 4) {
			Chip_I2S_Send(LPC_I2S0, out);
		}

		if(gain < 1.0) gain *= 1.001;
	}
}


/*
 * End
 */
