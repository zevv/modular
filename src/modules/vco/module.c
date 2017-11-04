#include <math.h>
#include "module.h"
#include "osc.h"
#include "ctl.h"
#include "fir.h"

#define OVERSAMPLE 4

static struct osc osc1, osc2, osc3, osc4;
static float freq, vol, fm;
static bool bandlimit;
static struct fir fir3, fir4;

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



static void on_set_bandlimit(void)
{
	osc_set_type(&osc2, bandlimit ? OSC_TYPE_TRIANGLE : OSC_TYPE_TRIANGLE_NAIVE);
	osc_set_type(&osc3, bandlimit ? OSC_TYPE_PULSE : OSC_TYPE_PULSE_NAIVE);
	osc_set_type(&osc4, bandlimit ? OSC_TYPE_SAW : OSC_TYPE_SAW_NAIVE);
}


void mod_init(void)
{
	osc_init(&osc1, SRATE);
	osc_init(&osc2, SRATE);
	osc_init(&osc3, SRATE * OVERSAMPLE);
	osc_init(&osc4, SRATE * OVERSAMPLE);

	osc_set_type(&osc1, OSC_TYPE_SIN);
	osc_set_type(&osc2, OSC_TYPE_TRIANGLE);
	osc_set_type(&osc3, OSC_TYPE_PULSE);
	osc_set_type(&osc4, OSC_TYPE_SAW);

	ctl_bind_pot(6, &freq, NULL, POT_SCALE_LOG, 10, 20000);
	ctl_bind_pot(4, &vol, NULL, POT_SCALE_LOG, 0, 1);
	ctl_bind_pot(5, &fm, NULL, POT_SCALE_LOG, 0, 10);
	ctl_bind_switch(7, &bandlimit, on_set_bandlimit);

	fir_init(&fir3, filter_taps, FILTER_TAP_NUM);
	fir_init(&fir4, filter_taps, FILTER_TAP_NUM);
}


void mod_run(float *fin, float *fout)
{
	float f = freq;

	float Ufm = fm * fin[0] * 5;

	f = f * powf(2, Ufm);

	osc_set_freq(&osc1, f);
	osc_set_freq(&osc2, f);
	osc_set_freq(&osc3, f);
	osc_set_freq(&osc4, f);

	/* No need to oversample sine */

	fout[0] = osc_gen(&osc1) * vol;
	fout[1] = osc_gen(&osc2) * vol;

	int i;
	for(i=0; i<OVERSAMPLE; i++) {
		fir_load(&fir3, osc_gen(&osc3));
		fir_load(&fir4, osc_gen(&osc4));
	}
		
	fout[2] = fir_calc(&fir3) * vol;
	fout[3] = fir_calc(&fir4) * vol;
}


struct module mod = {
	.init = mod_init,
	.run_float = mod_run,
};


/*
 * End
 */

