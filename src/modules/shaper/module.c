
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>


#include <string.h>
#include <math.h>
#include "module.h"
#include "osc.h"
#include "biquad.h"
#include "ctl.h"
#include "fir.h"
#include "tabread.h"


#define FILTER_TAP_NUM 59

static float filter_taps[FILTER_TAP_NUM] = {
	-0.005832109308329536, -0.008638427635512525, -0.01167350812107773,
	-0.011979136344676107, -0.008070742845749669, 0.00027249198403681165,
	0.011499814217631762, 0.022470953320626697, 0.029430776810863202,
	0.029477412840522205, 0.021964480879001134, 0.009177883772194244,
	-0.004177001619150875, -0.012657156896127944, -0.01236070380566301,
	-0.002872609911170217, 0.012051185768332447, 0.025574640953256653,
	0.03043849975249719, 0.02223813829798743, 0.001976280834505929,
	-0.02331288689846333, -0.04240177340038268, -0.0437306181998531,
	-0.01995604974755635, 0.028569252911858304, 0.09286178831397998,
	0.15749111100439436, 0.2052744277627895, 0.22287367251308382,
	0.2052744277627895, 0.15749111100439436, 0.09286178831397998,
	0.028569252911858304, -0.01995604974755635, -0.0437306181998531,
	-0.04240177340038268, -0.02331288689846333, 0.001976280834505929,
	0.02223813829798743, 0.03043849975249719, 0.025574640953256653,
	0.012051185768332447, -0.002872609911170217, -0.01236070380566301,
	-0.012657156896127944, -0.004177001619150875, 0.009177883772194244,
	0.021964480879001134, 0.029477412840522205, 0.029430776810863202,
	0.022470953320626697, 0.011499814217631762, 0.00027249198403681165,
	-0.008070742845749669, -0.011979136344676107, -0.01167350812107773,
	-0.008638427635512525, -0.005832109308329536
};


float shape[] = {
	-1.0, -0.99902, -0.99862, -0.99805, -0.99724, -0.99609, -0.99448, -0.99219,
	-0.98895, -0.98438, -0.97790, -0.96875, -0.95581, -0.93750, -0.91161,
	-0.87500, -0.82322, -0.75000, -0.64645, -0.50000, -0.29289, 0.00000,
	0.29289, 0.50000, 0.64645, 0.75000, 0.82322, 0.87500, 0.91161, 0.93750,
	0.95581, 0.96875, 0.97790, 0.98438, 0.98895, 0.99219, 0.99448, 0.99609,
	0.99724, 0.99805, 0.99862, 0.99902, 
};


static struct biquad hp;
static struct osc osc;
static struct fir fir1, fir2;
static float f, g;
static bool oversample;

static void on_set(void)
{
	osc_set_freq(&osc, f);
}


void mod_init(void)
{
	osc_init(&osc, SRATE);
	osc_set_freq(&osc, 220);
	osc_set_type(&osc, OSC_TYPE_SIN);

	ctl_bind_pot(6, &g, on_set, POT_SCALE_LOG, 0, 150);
	ctl_bind_pot(4, &f, on_set, POT_SCALE_LOG, 100, 10000);
	ctl_bind_switch(7, &oversample, on_set);

	fir_init(&fir1, filter_taps, FILTER_TAP_NUM);
	fir_init(&fir2, filter_taps, FILTER_TAP_NUM);

	biquad_init(&hp, SRATE);
	biquad_config(&hp, BIQUAD_TYPE_HP, 25, 0.707);
}


float dist(float v)
{
	if(v < -1.0) v = -1.0;
	if(v > +1.0) v = +1.0;
	v = read2(shape, sizeof(shape)/sizeof(shape[0]), (v+1)*0.48);
	return v;
}


void mod_run(float *fin, float *fout)
{
	float v = osc_gen(&osc);
	v = fin[0];
	fout[0] = fout[1] = v;

	v = biquad_run(&hp, v);
	v = v * g;
	fir_load(&fir1, v);
		
	if(oversample) {
		int i;
		for(i=0; i<4; i++) {
			v = fir_calc_poly(&fir1, 4, i);
			v = dist(v);
			fir_load(&fir2, v);
		}
		
		v = fir_calc(&fir2);

	} else {
		v = dist(v);
	}

	fout[0] = fout[1] = fout[2] = fout[3] = v * 0.1;
}


struct module mod = {
	.init = mod_init,
	.run_float = mod_run,
};


#ifdef SIM

int main(void)
{
	mod_init();

	float fin[32];
	float fout[32];
	size_t i;

	fin[4] = 1.0;
	fin[5] = 1.0;
	fin[6] = 1.0;

	for(i=0; i<4800; i++) {
		fin[0] = cos(i * 6.28 / SRATE) * 1.0;
		mod_run(fin, fout);
		printf("%f\n", fout[0]);
	}

	return 0;
}

#endif

/*
 * End
 */
