
#include <string.h>
#include <stdlib.h>

#include "module.h"

static int q;
static float c1;
static float c2;
static float c3;

float prand(void)
{
	static uint32_t r = 22222;
	r = (r * 196314165) + 907633515;
	return r / (float)UINT32_MAX;
}


float gauss(void)
{

    float random = prand();
    float noise = (2.f * ((random * c2) + (random * c2) + (random * c2)) - 3.f * (c2 - 1.f)) * c3;
    return noise;
}


void mod_init(void)
{
	q = 15;
	c1 = (1 << q) - 1;
	c2 = ((int)(c1 / 3)) + 1;
	c3 = 1.f / c1;
}


void mod_run(float *fin, float *fout)
{
	fout[0] = prand() * 2. - 1.;
	fout[1] = prand() * 2. - 1.;
}


struct module mod = {
	.init = mod_init,
	.run_float = mod_run,
};


/*
 * End
 */
