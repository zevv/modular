
#include <stdlib.h>
#include <math.h>

#include "module.h"
#include "board.h"
#include "lib/osc.h"
#include "os/printd.h"
#include "os/uart.h"

struct mod_waveshaper {
	bool enable;
};


static void *fn_new(uint32_t srate)
{
	struct mod_waveshaper *mo;
	mo = malloc(sizeof *mo);
	return mo;
}


static void fn_audio(struct module *mod, float *sig)
{
	struct mod_waveshaper *mo = mod->data;

	if(mo->enable) {
		sig[1] = sig[0] * 0.9 + sqrtf(sig[0]) * 0.1;
	} else {
		sig[1] = sig[0];
	}
}


static void fn_control(struct module *mod, size_t idx, float val)
{
	struct mod_waveshaper *mo = mod->data;

	if(idx == 0) {
		mo->enable = val;
	}
}

MODULE_REGISTER {
	.name = "waveshaper",
	.fn_new = fn_new,
	.fn_audio = fn_audio,
	.fn_control = fn_control,
};

/*
 * End
 */
