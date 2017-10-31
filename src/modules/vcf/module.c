
#include <string.h>
#include <stdlib.h>

#include "module.h"
#include "biquad.h"
#include "pot.h"


struct biquad f1, f2;
struct pot pot_freq, pot_Q;


void mod_init(void)
{
	biquad_init(&f1, SRATE);
	biquad_init(&f2, SRATE);
	pot_init(&pot_freq, POT_SCALE_LOG, 10, 15000);
	pot_init(&pot_Q, POT_SCALE_LOG, 0.5, 5.0);
}


void mod_run(float *fin, float *fout)
{
	float Q = pot_read(&pot_Q, fin[4]);
	float f = pot_read(&pot_freq, fin[6]);

	biquad_config(&f1, BIQUAD_TYPE_LP, f, Q * 0.707);
	biquad_config(&f2, BIQUAD_TYPE_LP, f, Q * 1.365);

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
