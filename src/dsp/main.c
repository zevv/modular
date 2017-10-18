
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "chip.h"
#include "module.h"
#include "../core/shared.h"

void M0APP_IRQHandler(void)
{
	Chip_CREG_ClearM0AppEvent();
}



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


static const float scale = 1.0 / 32767.0;

void main(void)
{
	LPC_GPIO_PORT->B[1][11] = 0;

	NVIC_EnableIRQ(M0APP_IRQn);
	
	mod_init();

	float gain = 0.00001;
	float fin[12];
	float fout[4];

	for(;;) {
		__WFI();

		shared->m4_busy = true;
		
		update_led();
	
		union sample s;
		s.u32 = shared->i2s_in;

		/* Convert to float -1.0 .. +1.0 */

		fin[ 0] = s.s16[0] * scale;
		fin[ 1] = s.s16[1] * scale;
		fin[ 4] = shared->adc_in[0] * scale;
		fin[ 5] = shared->adc_in[1] * scale;
		fin[ 6] = shared->adc_in[2] * scale;
		fin[ 7] = shared->adc_in[3] * scale;
		fin[ 8] = shared->adc_in[4] * scale;
		fin[ 9] = shared->adc_in[5] * scale;
		fin[10] = shared->adc_in[6] * scale;
		fin[11] = shared->adc_in[7] * scale;

		/* Run module */

		mod_run(fin, fout);

		/* Saturate and convert back to s16 */

		s.s16[0] = __SSAT((int)(fout[0] * 32767.0 * gain), 16);
		s.s16[1] = __SSAT((int)(fout[1] * 32767.0 * gain), 16);

		if(Chip_I2S_GetTxLevel(LPC_I2S0) < 4) {
			Chip_I2S_Send(LPC_I2S0, s.u32);
		}

		/* Handle fade in/out */

		if(shared->m4_state == M4_STATE_FADEIN) {
			if(gain < 1.0) {
				gain *= 1.0003;
			} else {
				shared->m4_state = M4_STATE_RUNNING;
			}
		} else if(shared->m4_state == M4_STATE_FADEOUT) {
			if(gain > 0.00001) {
				gain *= 0.9997;
			} else {
				shared->m4_state = M4_STATE_SILENT;
			}
		}

		shared->m4_busy = false;
		shared->m4_ticks ++;
	}
}


/*
 * End
 */
