
#include "../../dsp/module.h"
#include "osc.h"

static struct osc osc1, osc2;

void mod_init(void)
{
	osc_init(&osc1, 48000);
	osc_init(&osc2, 48000);
	osc_set_freq(&osc1, 400);
	osc_set_freq(&osc2, 403);
}


void mod_run(float *fin, float *fout)
{
	fout[0] = osc_gen_linear(&osc1) * 0.01;
	fout[1] = osc_gen_linear(&osc2) * 0.01;
}


/*
 * End
 */

