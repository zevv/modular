
#include <string.h>
#include <stdlib.h>

#include "../../dsp/module.h"

	

void mod_init(void)
{
}


void mod_run(float *fin, float *fout)
{
	fout[0] = fin[0];
	fout[1] = fin[1];
	fout[2] = fin[2];
	fout[3] = fin[3];
}


/*
 * End
 */
