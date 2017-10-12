
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "chip.h"
#include "module.h"

void M0APP_IRQHandler(void)
{
	Chip_CREG_ClearM0AppEvent();
}


uint32_t *i2s_val = (void *)0x1001fc00;


union sample {
	uint32_t u32;
	int16_t s16[2];
};


void main(void)
{
	LPC_GPIO_PORT->B[1][11] = 0;

	NVIC_EnableIRQ(M0APP_IRQn);
	
	mod_init();

	int n = 0;
	float gain = 0.00001;

	for(;;) {
		__WFI();
	
		LPC_GPIO_PORT->B[1][11] = (n++ & 0xf00) != 0;

		union sample s;
		s.u32 = *i2s_val;

		float fin[4] = { s.s16[0] / 32767.0, s.s16[1] / 32767.0, 0, 0 };
		float fout[4] = { 0, 0, 0, 0} ;

		mod_run(fin, fout);

		s.s16[0] = fout[0] * 16000.0 * gain;
		s.s16[1] = fout[1] * 16000.0 * gain;

		if(Chip_I2S_GetTxLevel(LPC_I2S0) < 4) {
			Chip_I2S_Send(LPC_I2S0, s.u32);
		}

		if(gain < 1.0) gain += 0.00005;
	}
}


/*
 * End
 */
