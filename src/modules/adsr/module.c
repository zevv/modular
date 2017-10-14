
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "adsr.h"
	
ADSR adsr;
bool trig;

void mod_init(void)
{
	trig = false;
	initADSR(&adsr);
}


void mod_run(float *fin, float *fout)
{
	if(!trig && fin[0] > 0) {
		gate(&adsr);
		trig = true;
	}

	if(trig && fin[0] < 0) {
		reset(&adsr);
		trig = false;
	}

	fout[0] = process(&adsr);
}


/*
 * End
 */
