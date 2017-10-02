#ifndef module_h
#define module_h

#include <stdint.h>
#include <stdlib.h>

#include "os/romtab.h"

#define MAX_SIG_COUNT 4
#define MAX_CTL_COUNT 8

struct module_descriptor;

enum ctl_type {
	CTL_SWITCH,
	CTL_POT_LIN,
	CTL_POT_LOG,
};


struct ctl_desc {
	enum ctl_type type;
	float prev;
	float *ptr;
	float min;
	float max;
};


struct module {
	void *data;
	struct module *next;
	struct module_descriptor *desc;
	struct ctl_desc ctl_desc[MAX_CTL_COUNT];
	float *sig_desc[MAX_SIG_COUNT];
};

struct module_descriptor {
	const char *name;
	void *(*fn_new)(uint32_t srate);
	void (*fn_audio)(struct module *mod, float *sig);
	void (*fn_control)(struct module *mod, size_t idx, float val);
	void (*fn_tick)(struct module *mod);
};

struct module *mod_new(const char *name, uint32_t srate);
void mod_map_sig(struct module *mod, int idx, float *ptr);
void mod_map_switch(struct module *mod, int idx, float *ptr);
void mod_map_pot(struct module *mod, int idx, enum ctl_type type, float *ptr, float min, float max);


void modules_do_audio(void);
void modules_do_tick(void);

#define MODULE_REGISTER ROMTAB(module_descriptor)

#endif
