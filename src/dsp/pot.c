
#include "pot.h"

void pot_init(struct pot *pot, enum pot_scale scale, float min, float max)
{
	pot->scale = scale;
	pot->min = min;
	pot->max = max;
	biquad_init(&pot->lf, SRATE);
	biquad_config(&pot->lf, BIQUAD_TYPE_LP, 40, 0.707);
}


float pot_read(struct pot *pot, float v)
{
	v = (v + 1.0) * 0.5;
	if(pot->scale == POT_SCALE_LOG) {
		v = v * v * v;
	}
	v = v * (pot->max - pot->min) + pot->min;
	return v;
}



/*
 * End
 */

