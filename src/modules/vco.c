
#include <stdlib.h>

#include "module.h"
#include "board.h"
#include "lib/osc.h"
#include "os/printd.h"
#include "os/uart.h"

struct mod_vco {
	struct osc osc;
	struct osc osc2;
	struct osc lfo;
	float volume;
	float freq;
	bool vibrato;
};


static void *fn_new(uint32_t srate)
{
	struct mod_vco *mo;
	mo = malloc(sizeof *mo);
	mo->volume = 0.5;
	osc_init(&mo->osc, 48000);
	osc_init(&mo->osc2, 48000);
	osc_init(&mo->lfo, 48000);

	osc_set_freq(&mo->lfo, 6.0);
	return mo;
}


static void fn_audio(struct module *mod, float *sig)
{
	struct mod_vco *mo = mod->data;

	float freq = mo->freq;

	if(mo->vibrato) {
		freq *= (1 + osc_gen_linear(&mo->lfo) * 0.04);
	}

	osc_set_freq(&mo->osc, freq);
	osc_set_freq(&mo->osc2, freq * 0.502);

	sig[0] = osc_gen_linear(&mo->osc) * 0.3 + osc_gen_linear(&mo->osc2) * mo->vibrato * 0.15;
}


static void fn_control(struct module *mod, size_t idx, float val)
{
	struct mod_vco *mo = mod->data;

	if(idx == 0) {
		osc_set_type(&mo->osc, val ? OSC_TYPE_SAW : OSC_TYPE_SIN);
	}

	if(idx == 1) {
		mo->freq = val;
	}
	
	if(idx == 2) {
		mo->vibrato = val;
	}
}

MODULE_REGISTER {
	.name = "vco",
	.fn_new = fn_new,
	.fn_audio = fn_audio,
	.fn_control = fn_control,
};

/*
 * End
 */
