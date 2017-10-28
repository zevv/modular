#ifndef shared_h
#define shared_h

#include <stdbool.h>
#include <stdint.h>

/* Shared data at last 1K block of RAM2 */


enum m4_state {
	M4_STATE_HALT,
	M4_STATE_FADEIN,
	M4_STATE_RUNNING,
	M4_STATE_FADEOUT,
	M4_STATE_SILENT,
};

struct shared {
	struct {
		float min;
		float max;
	} level[16];
	int32_t i2s_in[4];
	int32_t i2s_out[4];
	int32_t adc_in[8];
	uint32_t m4_load;
	enum m4_state m4_state;
	int debug;
};

static volatile struct shared *shared = (struct shared *)0x10091800;

#endif
