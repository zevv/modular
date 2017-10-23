#include <math.h>
#include "../../dsp/module.h"
#include "osc.h"

static struct osc osc1, osc2, osc3, osc4, lfo;

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
	osc_set_type(&osc2, OSC_TYPE_SAW_NAIVE);

	osc_init(&lfo, SRATE);
	osc_set_type(&lfo, OSC_TYPE_SAW_NAIVE);
	osc_set_freq(&lfo, 0.5);
}


void mod_run(float *fin, float *fout)
{
	float f = fin[6] * 100 + 440;
	float g = (fin[4] + 1) * 0.03;

	if(g < -0.1) g = -0.1;
	if(g >  0.1) g = 0.1;

	osc_set_freq(&osc1, f);
	osc_set_freq(&osc2, 250);

	float o = osc_gen(&osc1) * g;

	fout[0] = fin[5];
	fout[1] = o;
}


/*
 * End
 */

