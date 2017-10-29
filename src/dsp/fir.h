#ifndef fir_h
#define fir_h

struct fir {
	float s[512];
	const float *taps;
	int ntaps;
	int head;
};


void fir_init(struct fir *fir, const float *taps, int ntaps);


inline void fir_load(struct fir *fir, float fin)
{
	fir->s[fir->head] = fin;
	fir->head ++;
	if(fir->head == fir->ntaps) {
		fir->head = 0;
	}
}


inline float fir_calc(struct fir *fir)
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


inline float fir_calc_poly(struct fir *fir, int n, int m)
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


#endif

