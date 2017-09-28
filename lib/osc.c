
#include <math.h>

#include "os/printd.h"

#include "lib/sintab.h"
#include "lib/osc.h"


void osc_init(struct osc *osc, float freq, float srate)
{
	osc->phase = 0;
	osc->srate = srate;
	osc_set_freq(osc, freq);
}


void osc_set_freq(struct osc *osc, float freq)
{
	osc->dphase = freq * SINTAB_SIZE / osc->srate;
}


/*
 * Nearest neighbour sine table lookup
 */

float osc_gen_nearest(struct osc *osc)
{
	int i = osc->phase;
	float val = sintab[i];
	osc->phase += osc->dphase;
	while(osc->phase >= SINTAB_SIZE) {
		osc->phase -= SINTAB_SIZE;
	}

	return val;
}


/*
 * Linear interpolation sine table lookup
 */

float osc_gen_linear(struct osc *osc)
{
	int i = osc->phase;
	int j = i+1;
	float a0 = osc->phase - i;
	float a1 = 1.0 - a0;

	if(j >= SINTAB_SIZE) j -= SINTAB_SIZE;
	float v0 = sintab[i];
	float v1 = sintab[j];

	float val = v0 * a1 + v1 * a0;

	osc->phase += osc->dphase;
	while(osc->phase >= SINTAB_SIZE) {
		osc->phase -= SINTAB_SIZE;
	}

	return val;
}


/*
 * End
 */

