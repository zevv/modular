#ifndef img_h
#define img_h

#include <stdint.h>

struct img {
	uint16_t w;
	uint16_t h;
	uint8_t data[];
};

#endif
