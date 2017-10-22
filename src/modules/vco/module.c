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
	osc_init(&lfo, SRATE);
	osc_set_type(&lfo, OSC_TYPE_SIN);
	osc_set_freq(&lfo, 0.2);
}


void mod_run(float *fin, float *fout)
{

	float f = fin[4] * 300 + 310 + fin[1] * fin[6] * 500;

	f = osc_gen(&lfo) * 2000 + 2010;

	f = 220;

	osc_set_freq(&osc1, f);
	osc_set_freq(&osc2, f * 0.503);
	osc_set_freq(&osc3, f);
	osc_set_freq(&osc4, f * 0.503);

	fout[0] = fout[1] = osc_gen(&osc1) * 0.50;
}


/*
 * End
 */

