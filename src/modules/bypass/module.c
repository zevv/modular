
#include <string.h>
#include <stdlib.h>

#include "../../dsp/module.h"
#include "lpc_chip_43xx/inc/cmsis.h"
#include "arm_math.h"



void mod_init(void)
{
	module_mode = MODULE_MODE_INT;
}


void mod_run(float *in, float *out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = in[3];
}

void mod_run_int(int16_t *in, int16_t *out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = in[3];
}


/*
 * End
 */
