
#include <string.h>
#include <stdlib.h>

#include "module.h"
#include "../../core/shared.h"
#include "biquad.h"
#include "ctl.h"


static struct biquad f1, f2;

static float freq;
static float Q;

void on_pot(void)
{
	biquad_config(&f1, BIQUAD_TYPE_LP, freq, Q * 0.707);
	biquad_config(&f2, BIQUAD_TYPE_LP, freq, Q * 1.365);
}


void mod_init(void)
{
	biquad_init(&f1, SRATE);
	biquad_init(&f2, SRATE);
	ctl_bind_pot(4, &Q, on_pot, POT_SCALE_LOG, 0.5, 15.0);
	ctl_bind_pot(6, &freq, on_pot, POT_SCALE_LOG, 10, 15000.0);
}


void mod_run(float *fin, float *fout)
{
	float v = fin[0];

	v = biquad_run(&f1, v);
	v = biquad_run(&f2, v);

	fout[0] = fout[1] = v / Q;
}

struct module mod = {
	.init = mod_init,
	.run_float = mod_run,
};



/*
 * End
 */
