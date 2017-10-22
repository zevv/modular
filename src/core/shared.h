#ifndef shared_h
#define shared_h

/* Shared data at last 1K block of RAM2 */


enum m4_state {
	M4_STATE_HALT,
	M4_STATE_FADEIN,
	M4_STATE_RUNNING,
	M4_STATE_FADEOUT,
	M4_STATE_SILENT,
};

struct shared {
	int32_t i2s_in[0];
	int32_t adc_in[8];
	uint32_t m4_ticks;
	bool m4_busy;
	enum m4_state m4_state;
};

static volatile struct shared *shared = (struct shared *)0x10091c80;

#endif
