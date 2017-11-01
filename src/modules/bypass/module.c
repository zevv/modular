
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "module.h"
#include "lpc_chip_43xx/inc/cmsis.h"
#include "arm_math.h"
#include "../../core/shared.h"


void mod_run_int(int16_t *in, int16_t *out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = in[3];
}



struct module mod = {
	.run_int16 = mod_run_int,
};


/*
 * End
 */
