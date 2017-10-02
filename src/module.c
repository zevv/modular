
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "module.h"
#include "board.h"
#include "uart.h"
#include "os/printd.h"


static struct module *module_list;


static void module_do_audio(struct module *mod)
{
	float sig[MAX_SIG_COUNT];
	size_t i;

	for(i=0; i<MAX_SIG_COUNT; i++) {
		float *ptr = mod->sig_desc[i];
		if(ptr) {
			sig[i] = *ptr;
		}
	}

	mod->desc->fn_audio(mod, sig);

	for(i=0; i<MAX_SIG_COUNT; i++) {
		float *ptr = mod->sig_desc[i];
		if(ptr) {
			*ptr = sig[i];
		}
	}
}


static void module_do_control(struct module *mod)
{
	size_t i;

	for(i=0; i<MAX_CTL_COUNT; i++) {

		struct ctl_desc *sd = &mod->ctl_desc[i];
		if(sd->ptr == NULL) continue;

		float val = *sd->ptr;
		float delta = fabs(val - sd->prev);
		if(delta < 0.005) continue;

		sd->prev = val;
		float val2 = 0;
		if(sd->type == CTL_SWITCH) {
			val2 = val >0;
		} else if(sd->type == CTL_POT_LIN) {
			val2 = (val * 0.5 + 0.5) * (sd->max - sd->min) + sd->min;
		} else if(sd->type == CTL_POT_LOG) {
			val2 = (expf(val) - 0.367879441) * 0.4254590641;
			val2 = val2 * (sd->max - sd->min) + sd->min;
		}
		mod->desc->fn_control(mod, i, val2);
	}
}


void modules_do_audio(void)
{
	struct module *mod = module_list;
	while(mod) {
		module_do_audio(mod);
		module_do_control(mod);
		mod = mod->next;
	}
}


void modules_do_tick(void)
{
	struct module *mod = module_list;
	while(mod) {
		if(mod->desc->fn_tick) {
			mod->desc->fn_tick(mod);
		}
		mod = mod->next;
	}
}




struct module *mod_new(const char *name, uint32_t srate)
{
	extern struct module_descriptor __start_module_descriptor;
	extern struct module_descriptor __stop_module_descriptor;
	struct module_descriptor *desc = &__start_module_descriptor;

	while(desc < &__stop_module_descriptor) {

		if(strcmp(desc->name, name) == 0) {

			printd("- %s\n", name);

			/* Create module */

			struct module *mod;
			mod = calloc(sizeof *mod, 1);
			mod->desc = desc;
			mod->data = desc->fn_new(srate);

			/* Add to global linked list */

			mod->next = module_list;
			module_list = mod;

			return mod;
		}
		desc ++;
	}
	return NULL;
}


void mod_map_sig(struct module *mod, int idx, float *ptr)
{
	mod->sig_desc[idx] = ptr;
}


void mod_map_switch(struct module *mod, int idx, float *ptr)
{
	struct ctl_desc *sd = &mod->ctl_desc[idx];
	sd->type = CTL_SWITCH;
	sd->ptr = ptr;
}


void mod_map_pot(struct module *mod, int idx, enum ctl_type type, float *ptr, float min, float max)
{
	struct ctl_desc *sd = &mod->ctl_desc[idx];
	sd->type = type;
	sd->ptr = ptr;
	sd->min = min;
	sd->max = max;
}

/*
 * End
 */
