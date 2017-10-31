#include <math.h>
#include "module.h"
#include "osc.h"
#include "biquad.h"
#include "pot.h"

static struct osc osc1, osc2, osc3, osc4;
static float freq, vol, fm;


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

	pot_bind(6, &freq, NULL, POT_SCALE_LOG, 10, 10000);
	pot_bind(4, &vol, NULL, POT_SCALE_LOG, 0, 1);
	pot_bind(5, &fm, NULL, POT_SCALE_LOG, 0, 10);
}


void mod_run(float *fin, float *fout)
{
	float f = freq * (1 + fin[3] * fm);

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

