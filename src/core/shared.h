#ifndef shared_h
#define shared_h

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#define SHARED_LOG_BUF_SIZE 1024

/* Shared data at the end of RAM2 */


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
	struct {
		uint8_t buf[SHARED_LOG_BUF_SIZE];
		volatile size_t head;
		volatile size_t tail;
	} log;
	void (*logd)(const char *str, ...);
};

static struct shared *shared = (struct shared *)0x10091800;

#endif
