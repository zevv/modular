
#include <stdlib.h>

#include "os/board.h"
#include "lib/biquad.h"
#include "lib/osc.h"
#include "os/printd.h"
#include "module.h"

struct mod_vcf {
	int stages;
	float freq;
	float Q;
	struct biquad filt[3];
};


static void *fn_new(uint32_t srate)
{
	struct mod_vcf *mo;
	int i;

	mo = malloc(sizeof *mo);
	mo->freq = 1000;
	mo->Q = 0.7;
	mo->stages = 1;

	for(i=0; i<3; i++) {
		biquad_init(&mo->filt[i], srate);
	}

	return mo;
}


static void fn_audio(struct module *mod, float *sig)
{
	struct mod_vcf *mo = mod->data;

	float val = sig[0];

	int i;
	for(i=0; i<3; i++) {
		float dummy;
		if(i < mo->stages) {
			biquad_run(&mo->filt[i], val, &val);
		} else {
			biquad_run(&mo->filt[i], val, &dummy);
		}
	}

	sig[1] = val;
}


static void fn_control(struct module *mod, size_t idx, float val)
{
	struct mod_vcf *mo = mod->data;

	if(idx == 0) mo->freq = val;
	if(idx == 1) mo->stages = val ? 2 : 3;
	if(idx == 2) mo->Q = val;
		
	int i;
	for(i=0; i<3; i++) {
		biquad_config(&mo->filt[i], BIQUAD_TYPE_BP, mo->freq, mo->Q);
	}
}


MODULE_REGISTER {
	.name = "vcf",
	.fn_new = fn_new,
	.fn_audio = fn_audio,
	.fn_control = fn_control,
};


/*
 * End
 */
