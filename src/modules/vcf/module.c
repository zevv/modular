
#include <string.h>
#include <stdlib.h>

#include "module.h"
#include "../../core/shared.h"
#include "biquad.h"
#include "ctl.h"


static struct biquad f1, f2, f3, f4;

static float freq;
static float Q;
static bool type;


static void on_pot(void)
{
	enum biquad_type t = type ? BIQUAD_TYPE_HP : BIQUAD_TYPE_LP;
	biquad_config(&f1, t, freq, Q * 0.707);
	biquad_config(&f2, t, freq, Q * 1.365);
	biquad_config(&f3, t, freq, Q * 0.707);
	biquad_config(&f4, t, freq, Q * 1.365);
}


static void mod_init(void)
{
	biquad_init(&f1, SRATE);
	biquad_init(&f2, SRATE);
	biquad_init(&f3, SRATE);
	biquad_init(&f4, SRATE);
	ctl_bind_pot(4, &Q, on_pot, POT_SCALE_LOG, 0.5, 15.0);
	ctl_bind_pot(6, &freq, on_pot, POT_SCALE_LOG, 50, 15000.0);
	ctl_bind_switch(7, &type, on_pot);
}


static void mod_run_int(int16_t *in, int16_t *out)
{
	out[0] = biquad_run(&f1, biquad_run(&f2, in[0])) / Q;
	out[1] = biquad_run(&f3, biquad_run(&f4, in[1])) / Q;
}


struct module mod = {
	.init = mod_init,
	.run_int16 = mod_run_int,
};



/*
 * End
 */
