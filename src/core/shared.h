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
};

struct shared {

	struct {
		int32_t min;
		int32_t max;
	} level[16];

	int16_t in[12];
	int16_t out[4];

	uint32_t m4_load;
	enum m4_state m4_state;

	struct {
		uint8_t buf[SHARED_LOG_BUF_SIZE];
		size_t head;
		size_t tail;
	} log;
	void (*logd)(const char *str, ...);
};

#define shared ((struct shared *)0x10091800)

#endif
