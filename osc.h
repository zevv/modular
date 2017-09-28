#ifndef osc_h
#define osc_h

struct osc {
	float phase;
	float srate;
	float dphase;
};

void osc_init(struct osc *osc, float freq, float srate);
void osc_set_freq(struct osc *osc, float freq);
float osc_gen_nearest(struct osc *osc);
float osc_gen_linear(struct osc *osc);

#endif
