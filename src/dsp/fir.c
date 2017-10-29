
#include <string.h>

#include "fir.h"


void fir_init(struct fir *fir, const float *taps, int ntaps)
{
	memset(fir->s, 0, sizeof(fir->s));
	fir->head = 0;
	fir->ntaps = ntaps;
	fir->taps = taps;
}


void fir_load(struct fir *fir, float fin)
{
	fir->s[fir->head] = fin;
	fir->head ++;
	if(fir->head == fir->ntaps) {
		fir->head = 0;
	}
}


float fir_calc(struct fir *fir)
{
	float fout = 0.;

	int i;
	int j = fir->head;
	for(i=0; i<fir->ntaps; i++) {
		j = (j > 0) ? j-1 : fir->ntaps-1;
		fout += fir->s[j] * fir->taps[i];
	}

	return fout;
}


float fir_calc_poly(struct fir *fir, int n, int m)
{
	float fout = 0.;

	int i;
	int j = fir->head;
	for(i=m; i<fir->ntaps; i+=n) {
		j = (j > 0) ? j-1 : fir->ntaps-1;
		fout += fir->s[j] * fir->taps[i];
	}

	return fout;
}

/*
 * End
 */
