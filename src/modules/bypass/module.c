
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "module.h"
#include "lpc_chip_43xx/inc/cmsis.h"
#include "arm_math.h"
#include "../../core/shared.h"


#define FFT_SIZE 128

arm_rfft_instance_q15 fft;
int16_t in[FFT_SIZE];
int32_t win[FFT_SIZE];

void mod_init(void)
{
	module_mode = MODULE_MODE_INT;
	arm_rfft_init_q15(&fft, FFT_SIZE, 0, 1);

	int i;
	for(i=0; i<FFT_SIZE; i++) {
		win[i] = 16384 * sinf(M_PI * 2 * i / FFT_SIZE - M_PI/2) + 16384;
	}
}


void mod_run_int(int16_t *in, int16_t *out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = in[3];
}


void mod_bg(void)
{
	static int n = 0;

	if(n < FFT_SIZE) {
		in[n] = shared->in[0] * win[n] >> 15;
	}

	n ++;

	if(n == 2000) {
		int16_t out[FFT_SIZE*2];
		uint8_t spec[FFT_SIZE/2];
		arm_rfft_q15(&fft, in, out);

		int i;
		for(i=0; i<FFT_SIZE/2; i++) {
			float p1 = out[i*2+0] * 8;
			float p2 = out[i*2+1] * 8;
			float p = p1*p1 + p2*p2;
			if(p > 0) {
				if(1) {
					p = 8.68 * logf(sqrtf(p));
				} else {
					p = sqrtf(p) / 682;
				}
			}
			spec[i] = p;
		}

		shared->logd("\e[H");
		int y, x;
		for(y=96; y>=16; y-=4) {
			for(x=0; x<FFT_SIZE/2; x++) {
				shared->logd("%c", spec[x] > y ? '#' : ' ');
			}
			shared->logd("\e[0m\e[0K\n");
		}
		shared->logd("\e[0J");
		n = 0;
	}

}


struct module mod = {
	.init = mod_init,
	.run_int16 = mod_run_int,
	.run_bg = mod_bg,
};


/*
 * End
 */
