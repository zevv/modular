#ifndef lib_biquad_h
#define lib_biquad_h

struct biquad {
	double x1, x2;
	double y0, y1, y2;
	double b0_a0, b1_a0, b2_a0;
	double a1_a0, a2_a0;
	bool initialized;
	bool first;
};

enum biquad_type {
	BIQUAD_TYPE_LP,
	BIQUAD_TYPE_HP,
	BIQUAD_TYPE_BP,
	BIQUAD_TYPE_BS,
};

/*
 * Initialze biquad filter
 */

int biquad_init(struct biquad *bq);
int biquad_config(struct biquad *bq, enum biquad_type type, double freq, double Q);

/*
 * Run biquad filer with new sample input, produces new filtered sample
 */

int biquad_run(struct biquad *bq, double v_in, double *v_out);

#endif
