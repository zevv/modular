#include <math.h>
#include "module.h"
#include "osc.h"
#include "biquad.h"
#include "ctl.h"

static struct osc osc1, osc2, osc3, osc4;
static float freq, vol, fm;
static bool bandlimit;


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
	osc_init(&osc3, SRATE);
	osc_init(&osc4, SRATE);

	osc_set_type(&osc1, OSC_TYPE_SIN);
	osc_set_type(&osc2, OSC_TYPE_TRIANGLE);
	osc_set_type(&osc3, OSC_TYPE_PULSE);
	osc_set_type(&osc4, OSC_TYPE_SAW);

	ctl_bind_pot(6, &freq, NULL, POT_SCALE_LOG, 10, 20000);
	ctl_bind_pot(4, &vol, NULL, POT_SCALE_LOG, 0, 1);
	ctl_bind_pot(5, &fm, NULL, POT_SCALE_LOG, 0, 10);
	ctl_bind_switch(7, &bandlimit, on_set_bandlimit);
}


void mod_run(float *fin, float *fout)
{
	float f = freq;

	osc_set_freq(&osc1, f);
	osc_set_freq(&osc2, f);
	osc_set_freq(&osc3, f);
	osc_set_freq(&osc4, f);

	fout[0] = osc_gen(&osc1) * vol;
	fout[1] = osc_gen(&osc2) * vol;
	fout[2] = osc_gen(&osc3) * vol;
	fout[3] = osc_gen(&osc4) * vol;
}


struct module mod = {
	.init = mod_init,
	.run_float = mod_run,
};


/*
 * End
 */

