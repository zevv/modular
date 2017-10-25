
#include <string.h>
#include <stdlib.h>

#include "module.h"
#include "biquad.h"
#include "pot.h"


struct biquad f1;
struct pot pot_freq, pot_Q;


void mod_init(void)
{
	biquad_init(&f1, SRATE);
	pot_init(&pot_freq, POT_SCALE_LOG, 10, 5000);
	pot_init(&pot_Q, POT_SCALE_LOG, 0.5, 15.0);
}


void mod_run(float *fin, float *fout)
{
	float f = pot_read(&pot_freq, fin[4]);
	float q = pot_read(&pot_Q, fin[6]);
	biquad_config(&f1, BIQUAD_TYPE_LP, f, q);
	fout[0] = biquad_run(&f1, fin[0]);
}


/*
 * End
 */
