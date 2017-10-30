
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "chip.h"
#include "module.h"
#include "../core/shared.h"


enum module_mode module_mode = MODULE_MODE_FLOAT;

void M0APP_IRQHandler(void)
{
	Chip_CREG_ClearM0AppEvent();
}


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


static void update_level(int i, int32_t v)
{
	if(v > shared->level[i].max) shared->level[i].max = v;
	if(v < shared->level[i].min) shared->level[i].min = v;
}


static const float scale_i2s = -1.0 / 32768.0;
void (*logd)(const char *str, ...);


volatile float gain = 1.0;
static int16_t iin[12] = { 0 };
static int16_t iout[4] = { 0 };


void mod_run(float *fin, float *fout) __attribute__((weak));
void mod_run(float *fin, float *fout) {}

void mod_run_int(int16_t *in, int16_t *out) __attribute__((weak));
void mod_run_int(int16_t *in, int16_t *out) {}


void I2S0_IRQHandler(void)
{
	uint32_t t1 = 0x00ffffffu;
	SysTick->VAL = t1;

	if(Chip_I2S_GetRxLevel(LPC_I2S0) > 0) {
		iin[0] = Chip_I2S_Receive(LPC_I2S0) >> 16;
		iin[1] = Chip_I2S_Receive(LPC_I2S0) >> 16;
	}

	if(Chip_I2S_GetRxLevel(LPC_I2S1) > 0) {
		iin[2] = Chip_I2S_Receive(LPC_I2S1) >> 16;
		iin[3] = Chip_I2S_Receive(LPC_I2S1) >> 16;
	}

	if(module_mode == MODULE_MODE_FLOAT) {
		float fin[12];
		float fout[4] = { 0 };

		fin[0] = iin[0] * scale_i2s;
		fin[1] = iin[1] * scale_i2s;
		fin[2] = iin[2] * scale_i2s;
		fin[3] = iin[3] * scale_i2s;
		fin[4] = iin[4] * scale_i2s;
		fin[5] = iin[5] * scale_i2s;
		fin[6] = iin[6] * scale_i2s;
		fin[7] = iin[7] * scale_i2s;

		mod_run(fin, fout);

		iout[0] = fout[0] * gain;
		iout[1] = fout[1] * gain;
		iout[2] = fout[2] * gain;
		iout[3] = fout[3] * gain;
	} else {
		mod_run_int(iin, iout);
	}

	if(Chip_I2S_GetTxLevel(LPC_I2S0) < 4) {
		Chip_I2S_Send(LPC_I2S0, iout[0] << 16);
		Chip_I2S_Send(LPC_I2S0, iout[1] << 16);
		Chip_I2S_Send(LPC_I2S1, iout[2] << 16);
		Chip_I2S_Send(LPC_I2S1, iout[3] << 16);
	}
		
	uint32_t t2 = SysTick->VAL;
	shared->m4_load = t1-t2;
}


void main(void)
{
	NVIC_EnableIRQ(M0APP_IRQn);
	
	mod_init();

	logd = shared->logd;

	logd("M4 ready\n");
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
	SysTick->LOAD = 0x00ffffffu;
	
	NVIC_EnableIRQ(I2S0_IRQn);

	for(;;) {

		__DMB();
		__WFI();

		int i;

		for(i=0; i<4; i++) {
			update_level(i+0, iin[i]);
			update_level(i+4, iout[i]);
		}

		for(i=0; i<8; i++) {
			iin[i+4] = (shared->adc_in[i] - 0x8000);
			update_level(i+8, iin[i+4]);
		}


		/* Bookkeeping */

		update_led();

		/* Handle fade in/out */

		if(shared->m4_state == M4_STATE_FADEIN) {
			if(gain < 32767.0) {
				gain *= 1.0006;
			} else {
				shared->m4_state = M4_STATE_RUNNING;
			}
		} else if(shared->m4_state == M4_STATE_FADEOUT) {
			if(gain > 1.0) {
				gain *= 0.9994;
			} else {
				shared->m4_state = M4_STATE_HALT;
			}
		}

		if(shared->m4_state == M4_STATE_HALT) {
			NVIC_DisableIRQ(I2S0_IRQn);
			for(;;) __WFI();
		}
	}
}


/*
 * End
 */
