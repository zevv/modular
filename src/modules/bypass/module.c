
#include <string.h>
#include <stdlib.h>

#include "../../dsp/module.h"

	

void mod_init(void)
{
}


void mod_run(float *fin, float *fout)
{
	fout[0] = fout[1] = fin[7];
}


/*
 * End
 */
