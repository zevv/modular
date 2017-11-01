#ifndef ctl_h
#define ctl_h

#include <stdbool.h>
#include <stdlib.h>

#include "biquad.h"

enum pot_scale {
	POT_SCALE_LIN,
	POT_SCALE_LOG,
};


enum ctl_type {
	CTL_TYPE_POT,
	CTL_TYPE_SWITCH,
};

struct ctl {
	enum ctl_type type;
	float *out_float;
	bool *out_bool;
	int16_t *in;
	int16_t val;
	enum pot_scale scale;
	void (*fn)(void);
	float min;
	float max;
	struct biquad lp;
};


void ctl_bind_pot(size_t idx, float *out, void (*fn)(void), enum pot_scale scale, float min, float max);
void ctl_bind_switch(size_t idx, bool *out, void (*fn)(void));

void ctl_poll(void);

#endif

