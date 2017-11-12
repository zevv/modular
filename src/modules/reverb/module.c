
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "module.h"
#include "reverb.h"
#include "biquad.h"
#include "ctl.h"

static revmodel_t rev;
static struct biquad lp;

static float room, damp, wet;


static void on_pot(void)
{
	revmodel_set(&rev, REVMODEL_SET_ROOMSIZE | REVMODEL_SET_DAMPING, room, damp, 0, 0);
}


void mod_init(void)
{
	rev_init(&rev, 40100);
	ctl_bind_pot(4, &room, on_pot, POT_SCALE_LIN, 0.0, 1.0);
	ctl_bind_pot(6, &damp, on_pot, POT_SCALE_LIN, 1.0, 0.0);
	ctl_bind_pot(5, &wet,  on_pot, POT_SCALE_LIN, 0.0, 1.0);
	biquad_init(&lp, SRATE);
	biquad_config(&lp, BIQUAD_TYPE_HP, 20, 0.707);
}


void mod_run(float *fin, float *fout)
{
	float in = biquad_run(&lp, fin[0] + fin[1]);

	
	float a = wet;
	float b = 1.0 - wet;

	revmodel_process(&rev, in, &fout[0], &fout[1]);

	fout[0] = fout[0] * a + fin[0] * b;
	fout[1] = fout[1] * a + fin[0] * b;
}


struct module mod = {
	.name = "reverb",
	.init = mod_init,
	.run_float = mod_run,
};


/*
 * End
 */
