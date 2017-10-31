#ifndef module_h
#define module_h

#include <stdint.h>

enum module_mode {
	MODULE_MODE_FLOAT,
	MODULE_MODE_INT,
};

extern enum module_mode module_mode;

struct module {
	void (*init)(void);
	void (*run_float)(float *fin, float *fout);
	void (*run_int16)(int16_t *in, int16_t *out);
	void (*run_bg)(void);
};

extern struct module mod;

#endif

