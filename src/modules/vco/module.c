#include <math.h>
#include "module.h"
#include "osc.h"
#include "biquad.h"

static struct osc osc1, osc2, osc3, osc4, lfo;
static struct biquad lp1, lp2;

void mod_init(void)
{
	osc_init(&osc1, SRATE);
	osc_init(&osc2, SRATE);
	osc_init(&osc3, SRATE);
	osc_init(&osc4, SRATE);
	osc_set_freq(&osc1, 400);
	osc_set_freq(&osc2, 400 * 0.503);
	osc_set_freq(&osc3, 403);
	osc_set_freq(&osc4, 403 * 0.503);
	osc_set_type(&osc1, OSC_TYPE_SIN);
	osc_set_type(&osc2, OSC_TYPE_SIN);

	osc_init(&lfo, SRATE);
	osc_set_type(&lfo, OSC_TYPE_SAW_NAIVE);
	osc_set_freq(&lfo, 0.5);

	biquad_init(&lp1, SRATE);
	biquad_config(&lp1, BIQUAD_TYPE_LP, 50, 0.707);
	biquad_init(&lp2, SRATE);
	biquad_config(&lp2, BIQUAD_TYPE_LP, 50, 0.707);
}


void mod_run(float *fin, float *fout)
{
	float f = biquad_run(&lp1, fin[6]) * 4000 + 4010;
	float g = (biquad_run(&lp2, fin[4]) + 0.8) * 0.5;

	g = g * g;

	if(g < -0.0) g = -0.0;
	if(g >  1.0) g = 1.0;

	osc_set_freq(&osc1, f);
	osc_set_freq(&osc2, f*0.5 + 5);
	osc_set_freq(&osc3, f*1.0 + 5);
	osc_set_freq(&osc4, f*0.25 + 5);

	fout[0] = (osc_gen(&osc1) + osc_gen(&osc2)) * g * 0.5;
	fout[1] = (osc_gen(&osc3) + osc_gen(&osc4)) * g * 0.5;
}


/*
 * End
 */

