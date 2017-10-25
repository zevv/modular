#ifndef pot_h
#define pot_h

#include <stdbool.h>
#include "biquad.h"

enum pot_scale {
	POT_SCALE_LIN,
	POT_SCALE_LOG,
};

struct pot {
	float *in;
	enum pot_scale scale;
	float min;
	float max;
	struct biquad lf;
	float dead_zone;
	float dead_scale;
};


void pot_init(struct pot *pot, enum pot_scale scale, float min, float max);
float pot_read(struct pot *pot, float v);
void pot_set_dead_zone(struct pot *pot);

#endif

