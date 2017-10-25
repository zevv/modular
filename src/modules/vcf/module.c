
#include <string.h>
#include <stdlib.h>

#include "module.h"
#include "biquad.h"

	
struct biquad f1;

void mod_init(void)
{
	biquad_init(&f1, SRATE);
}


void mod_run(float *fin, float *fout)
{
	float f = (fin[4] + 1);
	f = f * f * 1000;
	float q = (fin[6] + 1) * 4;
	biquad_config(&f1, BIQUAD_TYPE_LP, f, q);
	fout[0] = biquad_run(&f1, fin[0]);
}


/*
 * End
 */
