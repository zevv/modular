
/*
 * Implementation of simple second order IIR biquad filters: low pass, high
 * pass, band pass and band stop.
 *
 * http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
 */

#include <math.h>
#include <stdbool.h>

#include "biquad.h"

#if 0

/* ID: these sin/cos approximations should save a few kb's on the binary size
 * to prevent linking in libm's sin and cos. I need to investigate if the
 * precision is sufficient first */

static float small_sin(float x)
{
	float x0 = x;
	float B =  4 / M_PI;
	float C = -4 /(M_PI*M_PI);

	while(x0 > M_PI) {
		x0 -= M_PI * 2;
	}

	while(x0 < -M_PI) {
		x0 += M_PI * 2;
	}

	float y = B * x0 + C * x0 * fabs(x0);
	float P = 0.225;

	y = P * (y * fabs(y) - y) + y;
	return y;
}


static float small_cos(float x)
{
	return small_sin(x + M_PI_2);
}

#endif


int biquad_init(struct biquad *bq)
{
	bq->first = true;
	bq->x1 = bq->x2 = 0.0;
	bq->y0 = bq->y1 = bq->y2 = 0.0;

	return 0;
}


int biquad_config(struct biquad *bq, enum biquad_type type, float freq, float Q)
{
	int r = 0;

	if((freq <= 0.0) || (freq >= 0.5)) {
		r = -1;
		goto out;
	}

	float a0 = 0.0, a1 = 0.0, a2 = 0.0;
	float b0 = 0.0, b1 = 0.0, b2 = 0.0;

	float w0 = 2.0 * M_PI * freq;
	float alpha = sin(w0) / (2.0*Q);
	float cos_w0 = cos(w0);

	switch(type) {

		case BIQUAD_TYPE_LP:
			b0 = (1.0 - cos_w0) / 2.0;
			b1 = 1.0 - cos_w0;
			b2 = (1.0 - cos_w0) / 2.0;
			a0 = 1.0 + alpha;
			a1 = -2.0 * cos_w0;
			a2 = 1.0 - alpha;
			break;

		case BIQUAD_TYPE_HP:
			b0 = (1.0 + cos_w0) / 2.0;
			b1 = -(1.0 + cos_w0);
			b2 = (1.0 + cos_w0) / 2.0;
			a0 = 1.0 + alpha;
			a1 = -2.0 * cos_w0;
			a2 = 1.0 - alpha;
			break;

		case BIQUAD_TYPE_BP:
			b0 = Q * alpha;
			b1 = 0.0;
			b2 = -Q * alpha;
			a0 = 1.0 + alpha;
			a1 = -2.0 * cos_w0;
			a2 = 1.0 - alpha;
			break;

		case BIQUAD_TYPE_BS:
			b0 = 1.0;
			b1 = -2.0 * cos_w0;
			b2 = 1.0;
			a0 = a2 = 1.0 + alpha;
			a1 = -2.0 * cos_w0;
			a2 = 1.0 - alpha;
			break;

		default:
			r = -1;
			goto out;
			break;
	}

	if(a0 == 0.0) {
		r = -1;
		goto out;
	}

	bq->b0_a0 = b0 / a0;
	bq->b1_a0 = b1 / a0;
	bq->b2_a0 = b2 / a0;
	bq->a1_a0 = a1 / a0;
	bq->a2_a0 = a2 / a0;
	bq->initialized = true;

out:
	return r;
}


int biquad_run(struct biquad *bq, float v_in, float *v_out)
{
	int r;

	if(bq->initialized) {

		float x0 = v_in;
		float y0;

		if(bq->first) {
			bq->y1 = bq->y2 = bq->x1 = bq->x2 = x0;
			bq->first = false;
		}

		y0 = bq->b0_a0 * x0 +
		     bq->b1_a0 * bq->x1 +
		     bq->b2_a0 * bq->x2 -
		     bq->a1_a0 * bq->y1 -
		     bq->a2_a0 * bq->y2;

		bq->x2 = bq->x1;
		bq->x1 = x0;

		bq->y2 = bq->y1;
		bq->y1 = y0;

		*v_out = y0;

		r = 0;
	} else {
		*v_out = v_in;
		r = -1;
	}

	return r;
}


/*
 * End
 */
