#ifndef module_h
#define module_h

#include <stdint.h>

enum module_mode {
	MODULE_MODE_FLOAT,
	MODULE_MODE_INT,
};

extern enum module_mode module_mode;

void mod_init(void);
void mod_run(float *fin, float *fout);
void mod_run_int(int16_t *in, int16_t *out);

#endif

