#ifndef pot_h
#define pot_h

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
};


void pot_init(struct pot *pot, enum pot_scale scale, float min, float max);
float pot_read(struct pot *pot, float v);

#endif

