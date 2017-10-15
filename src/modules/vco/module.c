
#include "../../dsp/module.h"
#include "osc.h"

static struct osc osc1, osc2, osc3, osc4;

void mod_init(void)
{
	osc_init(&osc1, 48000);
	osc_init(&osc2, 48000);
	osc_init(&osc3, 48000);
	osc_init(&osc4, 48000);
	osc_set_freq(&osc1, 400);
	osc_set_freq(&osc2, 400 * 0.503);
	osc_set_freq(&osc3, 403);
	osc_set_freq(&osc4, 403 * 0.503);
}


void mod_run(float *fin, float *fout)
{
	fout[0] = osc_gen(&osc1) * 0.01 +
	          osc_gen(&osc2) * 0.01;
	fout[1] = osc_gen(&osc3) * 0.01+
	          osc_gen(&osc4) * 0.01;
}


/*
 * End
 */

