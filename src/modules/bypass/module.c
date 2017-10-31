
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "../../dsp/module.h"
#include "lpc_chip_43xx/inc/cmsis.h"
#include "arm_math.h"
#include "osc.h"
#include "biquad.h"
#include "../../core/shared.h"


#define FFT_SIZE 64

arm_rfft_instance_q15 fft;
int16_t buf[FFT_SIZE];
int32_t win[FFT_SIZE];
int16_t spec[FFT_SIZE*2];
struct osc osc;
struct biquad hp;

void mod_init(void)
{
	module_mode = MODULE_MODE_INT;
	arm_rfft_init_q15(&fft, FFT_SIZE, 0, 1);
	osc_init(&osc, SRATE);
	osc_set_type(&osc, OSC_TYPE_SIN);
	osc_set_freq(&osc, SRATE / 8);
	biquad_init(&hp, SRATE);
	biquad_config(&hp, BIQUAD_TYPE_HP, 20, 0.707);

	int i;
	for(i=0; i<FFT_SIZE; i++) {
		win[i] = 16384 * sinf(M_PI * 2 * i / FFT_SIZE - M_PI/2) + 16384;
	}
}


void mod_run(float *in, float *out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = in[3];
}

void mod_run_int(int16_t *in, int16_t *out)
{
	static int n = 0;

	if(n < FFT_SIZE) {
		buf[n] = in[0] * win[n] >> 15;
	}

	n ++;

	if(n == 4000) {
		arm_rfft_q15(&fft, buf, spec);

		int i;
		for(i=0; i<FFT_SIZE; i+=2) {
			float p1 = spec[i+0] * 8;
			float p2 = spec[i+1] * 8;
			float p = p1*p1+p2*p2;
			if(p > 0) {
				if(1) {
					p = 8.68 * logf(sqrtf(p));
				} else {
					p = sqrtf(p) / 682;
				}
			}
			int j;
			for(j=15; j<96; j+=2) {
				shared->logd("%c", j<=p ? '#' : '.');
			}
			shared->logd(": %3d\n", -(int)(96-p));
		}
		shared->logd("\n");
		n = 0;
	}

	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = in[3];
}


/*
 * End
 */
