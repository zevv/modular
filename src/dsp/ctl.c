
#include <stdint.h>

#include "ctl.h"
#include "../core/shared.h"

#define MAX_POTS 12

static struct ctl ctl_list[MAX_POTS];
static size_t ctl_count = 0;



static struct ctl *ctl_init(size_t idx, enum ctl_type type)
{
	struct ctl *ctl = &ctl_list[ctl_count ++];
	ctl->type = type;
	ctl->in = &shared->in[idx];
	biquad_init(&ctl->lp, SRATE);
	biquad_config(&ctl->lp, BIQUAD_TYPE_LP, 100, 0.1);
	return ctl;
}


void ctl_bind_pot(size_t idx, float *out, void (*fn)(void), enum pot_scale scale, float min, float max)
{
	struct ctl *ctl = ctl_init(idx, CTL_TYPE_POT);

	ctl->out = out;
	ctl->fn = fn;
	ctl->scale = scale;
	ctl->min = min;
	ctl->max = max;
}


void ctl_bind_switch(size_t idx, bool *out, void (*fn)(void))
{
	struct ctl *ctl = ctl_init(idx, CTL_TYPE_SWITCH);

	ctl->in = &shared->in[idx];
	ctl->out = out;
	ctl->fn = fn;
}


void ctl_poll(void)
{
	size_t i;

	for(i=0; i<ctl_count; i++) {

		struct ctl *ctl = &ctl_list[i];

		int32_t val = biquad_run(&ctl->lp, *ctl->in);
		int32_t delta = 128;
		bool hit = false;

		if(val > ctl->val + delta) {
			ctl->val = val - delta;
			hit = true;
		}

		if(val < ctl->val - delta) {
			ctl->val = val + delta;
			hit = true;
		}

		if(hit) {
			if(ctl->type == CTL_TYPE_POT) {
				float v = ctl->val / 65536.0 + 0.5;
				if(ctl->scale == POT_SCALE_LOG) {
					v = v * v * v * v;
				}
				if(v < 0.0) v = 0.0;
				if(v > 1.0) v = 1.0;
				*(float *)ctl->out = v * (ctl->max - ctl->min) + ctl->min;
			}

			if(ctl->type == CTL_TYPE_SWITCH) {
				*(float *)ctl->out = ctl->val > 0;
			}

			if(ctl->fn) {
				ctl->fn();
			}
		}
	}
}


/*
 * End
 */

