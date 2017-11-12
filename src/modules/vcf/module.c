
#include <string.h>
#include <stdlib.h>

#include "module.h"
#include "../../core/shared.h"
#include "biquad.h"
#include "ctl.h"
#include "osc.h"


static struct biquad f1, f2;
static struct osc lfo;
static float freq;
static float lfo_freq;
static float Q;
static bool type;
static float fm;


static void on_pot(void)
{
	osc_set_freq(&lfo, lfo_freq);
}


static void mod_init(void)
{
	osc_init(&lfo, SRATE);
	biquad_init(&f1, SRATE);
	biquad_init(&f2, SRATE);
	ctl_bind_pot(4, &Q, on_pot, POT_SCALE_LOG, 0.5, 15.0);
	ctl_bind_pot(5, &fm, on_pot, POT_SCALE_LOG, 0, 5);
	ctl_bind_pot(6, &freq, on_pot, POT_SCALE_LOG, 50, 15000.0);
	ctl_bind_pot(9, &lfo_freq, on_pot, POT_SCALE_LOG, 0.5, 500);
	ctl_bind_switch(7, &type, on_pot);
}


static void mod_run_int(int16_t *in, int16_t *out)
{
	float f = freq * (1 + in[3] / 32767.0 * fm);
	if(f < 0) f = 0;
	if(f > SRATE / 2) f = SRATE / 2;
	enum biquad_type t = type ? BIQUAD_TYPE_HP : BIQUAD_TYPE_LP;
	biquad_config(&f1, t, f, Q * 0.707);
	biquad_config(&f2, t, f, Q * 1.365);

	out[0] = biquad_run(&f1, biquad_run(&f2, in[0])) / Q;
	out[3] = osc_gen(&lfo) * 32000;
}


struct module mod = {
	.name = "vcf",
	.init = mod_init,
	.run_int16 = mod_run_int,
};



/*
 * End
 */
