
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "../../dsp/module.h"
#include "reverb.h"

	
static revmodel_t rev;


void mod_init(void)
{
	rev_init(&rev, 24000);
}


void mod_run(float *fin, float *fout)
{
	float in = (fin[0] + fin[1]) * 0.5;

	revmodel_process(&rev, in, &fout[0], &fout[1]);
}


/*
 * End
 */
