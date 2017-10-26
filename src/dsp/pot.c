
#include "pot.h"


void pot_init(struct pot *pot, enum pot_scale scale, float min, float max)
{
	pot->scale = scale;
	pot->min = min;
	pot->max = max;
	pot->dead_zone = 0;
	biquad_init(&pot->lf, SRATE);
	biquad_config(&pot->lf, BIQUAD_TYPE_LP, 10, 0.1);
}


float pot_read(struct pot *pot, float v)
{
	v = biquad_run(&pot->lf, v);

	if(pot->dead_zone != 0.0) {
		if(v < -pot->dead_zone) {
			v = (v + pot->dead_zone) * pot->dead_scale;
		} else if(v > pot->dead_zone) {
			v = (v - pot->dead_zone) * pot->dead_scale;
		} else {
			v = 0;
		}
	}

	v = (v + 1.0) * 0.5;

	if(pot->scale == POT_SCALE_LOG) {
		v = v * v * v;
	}

	if(v < 0.0) v = 0.0;
	if(v > 1.0) v = 1.0;

	return v * (pot->max - pot->min) + pot->min;
}


void pot_set_dead_zone(struct pot *pot)
{
	pot->dead_zone = 0.1;
	pot->dead_scale = 1.0 / (1.0 - pot->dead_zone);
}


/*
 * End
 */

