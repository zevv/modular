
#include <string.h>

#include "fir.h"


void fir_init(struct fir *fir, const float *taps, int ntaps)
{
	memset(fir->s, 0, sizeof(fir->s));
	fir->head = 0;
	fir->ntaps = ntaps;
	fir->taps = taps;
}



/*
 * End
 */
