
#include <string.h>

#include "fir.h"


void fir_init(struct fir *fir, const float *taps, int ntaps)
{
	memset(fir->s, 0, sizeof(fir->s));
	fir->ntaps = ntaps;
	fir->taps = taps;
}


void fir_load(struct fir *fir, float fin)
{
	fir->s[fir->head++] = fin;
	if(fir->head > fir->ntaps) {
		fir->head = 0;
	}
}


float fir_calc(struct fir *fir)
{
	int p = fir->head;

	float fout = 0.;

	int i;
	for(i=0; i<fir->ntaps; i++) {
		p --;
		if(p < 0) {
			p += fir->ntaps;
		}
		fout += fir->s[p] * fir->taps[i];
	}

	return fout;
}

/*
 * End
 */
