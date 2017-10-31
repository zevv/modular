
#include <stdint.h>

#include "pot.h"
#include "../core/shared.h"

#define MAX_POTS 12

static struct pot pot_list[MAX_POTS];
static size_t pot_count = 0;


void pot_init(struct pot *pot, enum pot_scale scale, float min, float max)
{
	pot->scale = scale;
	pot->min = min;
	pot->max = max;
	pot->dead_zone = 0;
	biquad_init(&pot->lf, SRATE);
	biquad_config(&pot->lf, BIQUAD_TYPE_LP, 10, 0.1);
}


void pot_bind(size_t idx, float *out, void (*fn)(void), enum pot_scale scale, float min, float max)
{
	struct pot *pot = &pot_list[pot_count ++];

	pot_init(pot, scale, min, max);
	pot->in = &shared->in[idx];
	pot->out = out;
	pot->fn = fn;
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
		v = v * v * v * v;
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


void pot_poll(void)
{
	size_t i;

	for(i=0; i<pot_count; i++) {

		struct pot *pot = &pot_list[i];

		int32_t raw = *pot->in;
		int32_t delta = 512;
		bool hit = false;

		if(raw > pot->raw_prev + delta) {
			pot->raw_prev = raw - delta;
			hit = true;
		}

		if(raw < pot->raw_prev - delta) {
			pot->raw_prev = raw + delta;
			hit = true;
		}

		if(hit) {
			float v = pot->raw_prev / 65536.0 + 0.5;
			if(pot->scale == POT_SCALE_LOG) {
				v = v * v * v * v;
			}
			if(v < 0.0) v = 0.0;
			if(v > 1.0) v = 1.0;
			*pot->out = v * (pot->max - pot->min) + pot->min;

			if(pot->fn) {
				pot->fn();
			}
		}
	}
}


/*
 * End
 */

