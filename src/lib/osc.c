
#include <math.h>

#include "os/printd.h"

#include "lib/sintab.h"
#include "lib/osc.h"


void osc_init(struct osc *osc, float srate)
{
	osc->phase = 0;
	osc->srate = srate;
	osc_set_type(osc, OSC_TYPE_SIN);
	osc_set_dutycycle(osc, 0.5);
	osc_set_freq(osc, 440);
}


void osc_set_freq(struct osc *osc, float freq)
{
	osc->dphase = freq * SINTAB_SIZE / osc->srate;
}


void osc_set_type(struct osc *osc, enum osc_type type)
{
	osc->type = type;
}


void osc_set_dutycycle(struct osc *osc, float dt)
{
	osc->dutycycle = dt * SINTAB_SIZE;
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
	float val = 0;

	if(osc->type == OSC_TYPE_SIN) {

		int i = osc->phase;
		int j = i+1;
		float a0 = osc->phase - i;
		float a1 = 1.0 - a0;

		if(j >= SINTAB_SIZE) j -= SINTAB_SIZE;
		float v0 = sintab[i];
		float v1 = sintab[j];

		val = v0 * a1 + v1 * a0;
	}
	
	if(osc->type == OSC_TYPE_PULSE) {
		val = osc->phase < osc->dutycycle ? -1 : 1;
	}

	if(osc->type == OSC_TYPE_SAW) {
		val = 2.0 * osc->phase / SINTAB_SIZE - 1.0;
	}

	osc->phase += osc->dphase;
	while(osc->phase >= SINTAB_SIZE) {
		osc->phase -= SINTAB_SIZE;
	}

	return val;
}


/*
 * End
 */

