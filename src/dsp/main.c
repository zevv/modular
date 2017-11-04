
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "chip.h"
#include "module.h"
#include "ctl.h"
#include "../core/shared.h"


static void update_state(void);
static void update_led(void);

enum module_mode module_mode = MODULE_MODE_FLOAT;

void M0APP_IRQHandler(void)
{
	Chip_CREG_ClearM0AppEvent();
}


static void update_level(int i, int32_t v)
{
	if(v > shared->level[i].max) shared->level[i].max = v;
	if(v < shared->level[i].min) shared->level[i].min = v;
}


static const float scale_i2s = 1.0 / 32768.0;
static float inv_ticks_per_sample = 1000 * SRATE / 204e6;
void (*logd)(const char *str, ...);

static volatile float gain = 1.0;

static union sample {
	int16_t s16[2];
	uint32_t u32;
} s;


void I2S0_IRQHandler(void)
{
	uint32_t t1 = SysTick->VAL;

	if(Chip_I2S_GetRxLevel(LPC_I2S0) >= 2) {
		uint32_t v = Chip_I2S_Receive(LPC_I2S0);
		shared->in[0] = v >> 16;
		shared->in[1] = v & 0xffff;
	}

	if(Chip_I2S_GetRxLevel(LPC_I2S1) >= 2) {
		uint32_t v = Chip_I2S_Receive(LPC_I2S1);
		shared->in[2] = v >> 16;
		shared->in[3] = v & 0xffff;
	}

	if(mod.run_float) {
		float fin[12];
		float fout[4] = { 0 };

		fin[ 0] = -shared->in[ 0] * scale_i2s;
		fin[ 1] = -shared->in[ 1] * scale_i2s;
		fin[ 2] = -shared->in[ 2] * scale_i2s;
		fin[ 3] = -shared->in[ 3] * scale_i2s;
		fin[ 4] =  shared->in[ 4] * scale_i2s;
		fin[ 5] =  shared->in[ 5] * scale_i2s;
		fin[ 6] =  shared->in[ 6] * scale_i2s;
		fin[ 7] =  shared->in[ 7] * scale_i2s;
		fin[ 8] =  shared->in[ 8] * scale_i2s;
		fin[ 9] =  shared->in[ 9] * scale_i2s;
		fin[10] =  shared->in[10] * scale_i2s;
		fin[11] =  shared->in[11] * scale_i2s;

		mod.run_float(fin, fout);

		shared->out[0] = fout[0] * gain;
		shared->out[1] = fout[1] * gain;
		shared->out[2] = fout[2] * gain;
		shared->out[3] = fout[3] * gain;
	}
	
	if(mod.run_int16) {
		mod.run_int16((int16_t *)shared->in, (int16_t *)shared->out);
	}

	if(Chip_I2S_GetTxLevel(LPC_I2S0) < 4) {
		s.s16[0] = shared->out[1];
		s.s16[1] = shared->out[0];
		Chip_I2S_Send(LPC_I2S0, s.u32);
	}
	if(Chip_I2S_GetTxLevel(LPC_I2S1) < 4) {
		s.s16[0] = shared->out[3];
		s.s16[1] = shared->out[2];
		Chip_I2S_Send(LPC_I2S1, s.u32);
	}

	if(shared->scope.src && shared->scope.n < SHARED_SCOPE_SIZE) {
		shared->scope.buf[shared->scope.n++] = *shared->scope.src;
	}
		
	uint32_t t2 = SysTick->VAL;
	shared->m4_load = ((int32_t)t1-(int32_t)t2) * inv_ticks_per_sample;
}


void main(void)
{
	if(mod.init) {
		mod.init();
	}

	logd = shared->logd;

	logd("M4 ready\n");
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
	SysTick->LOAD = 0x00ffffffu;
	
	NVIC_EnableIRQ(I2S0_IRQn);

	for(;;) {

		__DMB();
		__WFI();

		if(mod.run_bg) {
			mod.run_bg();
		}

		int i;

		for(i=0; i<4; i++) {
			update_level(i+0, shared->in[i]);
			update_level(i+4, shared->out[i]);
		}

		for(i=0; i<8; i++) {
			update_level(i+8, shared->in[i+4]);
		}


		/* Bookkeeping */

		update_led();
		update_state();
		ctl_poll();
	}

}


static void update_state(void)
{
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
		for(;;) {
			__WFI();
			Chip_GPIO_SetPinState(LPC_GPIO_PORT, 1, 11, 0);
		}
	}
}


static void update_led(void)
{
	static const uint8_t breath[] = {
		0, 0, 0, 16, 32, 64, 128, 254, 254, 200, 150, 100, 60, 40, 30,
		30, 60, 120, 240, 230, 200, 170, 140, 110, 90, 70, 50, 40, 30,
		20, 10, 7, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	};

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




/*
 * End
 */
