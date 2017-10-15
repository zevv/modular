
#include <math.h>

#include "sintab.h"
#include "osc.h"


float dpw3(float t, float dt)
{
	static float x1 =0, x2 = 0;
	t += 0.5 + dt;
	if (t >= 1.) t -= 1.;

	float x = 2.*t - 1.;

	float x0 = x*x*x - x;
	float y = ((x0 - x1) - (x1 - x2)) / (24.*dt*dt);
	x2 = x1;
	x1 = x0;
	return y;
}

float poly_blep(float t, float dt)
{
	// 0 <= t < 1
	if (t < dt)
	{
		t /= dt;
		// 2 * (t - t^2/2 - 0.5)
		return t+t - t*t - 1.;
	}

	// -1 < t < 0
	else if (t > 1. - dt)
	{
		t = (t - 1.) / dt;
		// 2 * (t^2/2 + t + 0.5)
		return t*t + t+t + 1.;
	}

	// 0 otherwise
	else
	{
		return 0.;
	}
}



void osc_init(struct osc *osc, float srate)
{
	osc->phase = 0;
	osc->srate_inv = 1.0/srate;
	osc_set_type(osc, OSC_TYPE_SIN);
	osc_set_dutycycle(osc, 0.5);
	osc_set_freq(osc, 440);
}


void osc_set_freq(struct osc *osc, float freq)
{
	osc->dphase = freq * osc->srate_inv;
}


void osc_set_type(struct osc *osc, enum osc_type type)
{
	osc->type = type;
}


void osc_set_dutycycle(struct osc *osc, float dt)
{
	osc->dutycycle = dt;
}



/*
 * Linear interpolation sine table lookup
 */

float osc_gen_linear(struct osc *osc)
{
	float val = 0;


	if(osc->type == OSC_TYPE_SIN) {

		float p = osc->phase * SINTAB_SIZE;
		int i = p;
		int j = i+1;
		float a0 = p - i;
		float a1 = 1.0 - a0;

		if(j >= SINTAB_SIZE) j -= SINTAB_SIZE;
		float v0 = sintab[i];
		float v1 = sintab[j];

		val = v0 * a1 + v1 * a0;
	}
	
	if(osc->type == OSC_TYPE_PULSE) {
		val = osc->phase <= osc->dutycycle ? 1 : -1;
	}
	
	if(osc->type == OSC_TYPE_TRIANGLE) {
		if(osc->phase < 0.5) {
			val = osc->phase * 4 - 1;
		} else {
			val = 3 - osc->phase * 4;
		}
	}

	if(osc->type == OSC_TYPE_SAW) {
                osc->phase += 2 * osc->dt;
                if(osc->phase > 1-osc->dt) {
                        val = osc->phase - osc->phase/osc->dt + 1/osc->dt - 1;
			osc->phase -= 2;
                } else {
                        val = p;
                }
	}

	osc->phase += osc->dphase;
	while(osc->phase >= 1.0) {
		osc->phase -= 1.0;
	}

	return val;
}


#ifdef TEST

#include <stdio.h>

int main(void)
{
	static struct osc osc, lfo;
	osc_init(&lfo, 48000);

	osc_init(&osc, 48000);

	osc_set_freq(&lfo, 0.3);

	osc_set_type(&osc, OSC_TYPE_SAW);
	osc_set_freq(&osc, 1000);
	int i;

	for(i=0; i<480; i++) {
	
		float out;
		out = osc_gen_linear(&osc);
		printf("%f\n", out);
	}
}

#endif

/*
 * End
 */

