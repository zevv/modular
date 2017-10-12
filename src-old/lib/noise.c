
#include <stdlib.h>
#include <math.h>

#include "noise.h"

static float ranf()
{
	return rand() / (float)RAND_MAX;
}


float noise_gen(void)
{
	float x1, x2, w, y1;

	do {
		x1 = 2.0 * ranf() - 1.0;
		x2 = 2.0 * ranf() - 1.0;
		w = x1 * x1 + x2 * x2;
	} while ( w >= 1.0 );

	w = sqrtf( (-2.0 * logf( w ) ) / w );
	y1 = x1 * w;
	//y2 = x2 * w;
	return y1;
}

/*
 * End
 */
