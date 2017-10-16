#ifndef fir_h
#define fir_h

struct fir {
	float s[512];
	const float *taps;
	int ntaps;
	int head;
};


void fir_init(struct fir *fir, const float *taps, int ntaps);
void fir_load(struct fir *fir, float fin);
float fir_calc(struct fir *fir);

#endif

