
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

static double small_sin(double x)
{
	double x0 = x;
	double B =  4 / M_PI;
	double C = -4 /(M_PI*M_PI);

	while(x0 > M_PI) {
		x0 -= M_PI * 2;
	}

	while(x0 < -M_PI) {
		x0 += M_PI * 2;
	}

	double y = B * x0 + C * x0 * fabs(x0);
	double P = 0.225;

	y = P * (y * fabs(y) - y) + y;
	return y;
}


static double small_cos(double x)
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


int biquad_config(struct biquad *bq, enum biquad_type type, double freq, double Q)
{
	int r = 0;

	if((freq <= 0.0) || (freq >= 0.5)) {
		r = -1;
		goto out;
	}

	double a0 = 0.0, a1 = 0.0, a2 = 0.0;
	double b0 = 0.0, b1 = 0.0, b2 = 0.0;

	double w0 = 2.0 * M_PI * freq;
	double alpha = sin(w0) / (2.0*Q);
	double cos_w0 = cos(w0);

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


int biquad_run(struct biquad *bq, double v_in, double *v_out)
{
	int r;

	if(bq->initialized) {

		double x0 = v_in;
		double y0;

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
