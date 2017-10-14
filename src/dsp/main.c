
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


const uint8_t breath[] = {
	0, 0, 0, 16, 32, 64, 128, 254, 254, 200, 150, 100, 60, 40, 30, 30, 60,
	120, 240, 230, 200, 170, 140, 110, 90, 70, 50, 40, 30, 20, 10, 7, 5, 4,
	3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0,
};


static void update_led(void)
{
	static size_t i = 0;
	static size_t j = 0;
	static uint8_t v = 0;

	LPC_GPIO_PORT->B[1][11] = (j > v);

	if(++j >= 255) {
		j = 0;
		if(++i >= sizeof(breath)<<3) {
			i = 0;
		}
		v = breath[i >> 3];
	}
}


void main(void)
{
	LPC_GPIO_PORT->B[1][11] = 0;

	NVIC_EnableIRQ(M0APP_IRQn);
	
	mod_init();

	float gain = 0.00001;

	for(;;) {
		__WFI();
		
		update_led();
	
		union sample s;
		s.u32 = *i2s_val;

		/* Convert to float -1.0 .. +1.0 */

		float fin[4] = { s.s16[0] / 32767.0, s.s16[1] / 32767.0, 0, 0 };
		float fout[4] = { 0, 0, 0, 0} ;

		/* Run module */

		mod_run(fin, fout);

		/* Saturate and convert back to s16 */

		s.s16[0] = __SSAT((int)(fout[0] * 32767.0 * gain), 16);
		s.s16[1] = __SSAT((int)(fout[1] * 32767.0 * gain), 16);

		if(Chip_I2S_GetTxLevel(LPC_I2S0) < 4) {
			Chip_I2S_Send(LPC_I2S0, s.u32);
		}

		if(gain < 1.0) gain *= 1.0003;
	}
}


/*
 * End
 */
