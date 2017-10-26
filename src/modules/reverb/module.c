
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "../core/shared.h"
#include "module.h"
#include "reverb.h"
#include "pot.h"

	
static revmodel_t rev;
struct pot pot_room, pot_damp, pot_wet;


void mod_init(void)
{
	rev_init(&rev, 40100);
	pot_init(&pot_room, POT_SCALE_LIN, 0, 1);
	pot_init(&pot_damp, POT_SCALE_LIN, 1, 0);
	pot_init(&pot_wet,  POT_SCALE_LIN, 0, 1);
}


void mod_run(float *fin, float *fout)
{
	float in = (fin[0] + fin[1]);

	float room = pot_read(&pot_room, fin[4]);
	float damp = pot_read(&pot_damp, fin[6]);
	float wet  = pot_read(&pot_wet,  fin[5]);

	shared->debug = room * 1000;

	revmodel_set(&rev, REVMODEL_SET_ROOMSIZE | REVMODEL_SET_DAMPING, room, damp, 0, 0);
	
	float a = wet;
	float b = 1.0 - wet;

	revmodel_process(&rev, in, &fout[0], &fout[1]);

	fout[0] = fout[0] * a + fin[0] * b;
	fout[1] = fout[1] * a + fin[0] * b;
}


/*
 * End
 */
