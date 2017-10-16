#ifndef shared_h
#define shared_h

/* Shared data at last 1K block of RAM2 */

struct shared {
	uint32_t i2s_in;
	int32_t adc_in[8];
	uint32_t m4_ticks;
	bool m4_busy;
};

static volatile struct shared *shared = (struct shared *)0x10091c80;

#endif
