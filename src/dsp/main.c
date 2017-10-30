
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

	LPC_GPIO_PORT->B[0][14] = (j > v);

	if(++j >= 255) {
		j = 0;
		if(++i >= sizeof(breath)<<3) {
			i = 0;
		}
		v = breath[i >> 3];
	}
}


static void update_level(int i, float f)
{
	shared->level[i].max = (f > shared->level[i].max) ? f : shared->level[i].max;
	shared->level[i].min = (f < shared->level[i].min) ? f : shared->level[i].min;
}


static const float scale_i2s = -1.0 / 2147483648.0;
static const float scale_adc = -1.0 / 32767.0 * 1.2;
static const float offset_adc = 1.15;
void (*logd)(const char *str, ...);

void main(void)
{
	NVIC_EnableIRQ(M0APP_IRQn);
	
	mod_init();

	logd = shared->logd;

	logd("M4 ready\n");

	float gain = 0.00001;
	float fin[12];
	float fout[4];

	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
	SysTick->LOAD = 0x00ffffffu;
	uint32_t t1 = 0x00ffffffu;

	for(;;) {

		SysTick->VAL = t1;
		__WFI();
		uint32_t t2 = SysTick->VAL;

		/* Convert to float -1.0 .. +1.0 */

		int i;
		for(i=0; i<4; i++) {
			fin[i] = shared->i2s_in[i] * scale_i2s;
			update_level(i, fin[i]);
		}

		for(i=0; i<8; i++) {
			fin[i+4] = shared->adc_in[i] * scale_adc + offset_adc;
			update_level(i+8, fin[i+4]);
		}

		/* Run module */

		mod_run(fin, fout);

		/* Saturate at 24 bits and scale to 32 bits */

		for(i=0; i<4; i++) {
			shared->i2s_out[i] = __SSAT((int)(fout[i] * 8388608.0 * gain), 24) << 8;
			update_level(i+4, fout[i]);
		}

		if(Chip_I2S_GetTxLevel(LPC_I2S0) < 4) {
			Chip_I2S_Send(LPC_I2S0, shared->i2s_out[0]);
			Chip_I2S_Send(LPC_I2S0, shared->i2s_out[1]);
		}
		if(Chip_I2S_GetTxLevel(LPC_I2S1) < 4) {
			Chip_I2S_Send(LPC_I2S1, shared->i2s_out[2]);
			Chip_I2S_Send(LPC_I2S1, shared->i2s_out[3]);
		}

		/* Bookkeeping */

		update_led();

		/* Handle fade in/out */

		if(shared->m4_state == M4_STATE_FADEIN) {
			if(gain < 1.0) {
				gain *= 1.0006;
			} else {
				shared->m4_state = M4_STATE_RUNNING;
			}
		} else if(shared->m4_state == M4_STATE_FADEOUT) {
			if(gain > 0.00001) {
				gain *= 0.9994;
			} else {
				shared->m4_state = M4_STATE_SILENT;
			}
		}

		uint32_t t3 = SysTick->VAL;

		uint32_t load = 1000*(t2-t3)/(t1-t3);
		if(load > shared->m4_load) {
			shared->m4_load = load;
		}
	}
}


/*
 * End
 */
