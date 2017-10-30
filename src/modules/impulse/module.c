
#include <string.h>
#include <stdlib.h>

#include "../../dsp/module.h"
#include "lpc_chip_43xx/inc/cmsis.h"
#include "lpc_chip_43xx/inc/cmsis_gcc.h"
#include "arm_math.h"


static int16_t tap[] = {
	  -7, -3, -4, -5, -6, -6, -7, -7, -8, -8, -7, -7, -5, -4, -1, 2, 6, 10,
	  16, 22, 29, 38, 47, 57, 68, 80, 93, 106, 120, 134, 149, 163, 177,
	  190, 203, 215, 225, 234, 241, 246, 249, 249, 247, 242, 234, 222, 208,
	  190, 170, 147, 121, 93, 62, 30, -4, -38, -73, -108, -141, -174, -204,
	  -232, -256, -277, -293, -303, -309, -308, -300, -286, -265, -237,
	  -201, -159, -109, -52, 11, 80, 155, 234, 317, 404, 493, 584, 675,
	  766, 855, 942, 1025, 1104, 1177, 1244, 1304, 1356, 1399, 1433, 1458,
	  1473, 1478, 1473, 1458, 1433, 1399, 1356, 1304, 1244, 1177, 1104,
	  1025, 942, 855, 766, 675, 584, 493, 404, 317, 234, 155, 80, 11, -52,
	  -109, -159, -201, -237, -265, -286, -300, -308, -309, -303, -293,
	  -277, -256, -232, -204, -174, -141, -108, -73, -38, -4, 30, 62, 93,
	  121, 147, 170, 190, 208, 222, 234, 242, 247, 249, 249, 246, 241, 234,
	  225, 215, 203, 190, 177, 163, 149, 134, 120, 106, 93, 80, 68, 57, 47,
	  38, 29, 22, 16, 10, 6, 2, -1, -4, -5, -7, -7, -8, -8, -7, -7, -6, -6,
	  -5, -4, -3, -7, -7
};


#define NTAPS (sizeof(tap) / sizeof(tap[0]))

static int16_t tap2[NTAPS];
static int16_t sig[512];
static int head;



void mod_init(void)
{
	module_mode = MODULE_MODE_INT;
	int i;

	for(i=0; i<NTAPS; i+=2) {
		tap2[i+0] = tap[i+1];
		tap2[i+1] = tap[i+0];
	}
}


void mod_run_int(int16_t *in, int16_t *out)
{
	sig[head] = in[0];
	head = head + 1;
	if(head >= NTAPS) head = 0;

	int64_t acc = 0;
	int32_t tail = head;

	int t = tail;
	int i = 0 ;

	while(t > 16) {
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
	}

	while(t > 0) {
		acc += tap[i++] * sig[--t];
	}
	
	t = NTAPS;
	
	while(t > tail+16) {
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
		acc += tap[i++] * sig[--t];
	}
	
	while(t > tail) {
		acc += tap[i++] * sig[--t];
	}

	out[0] = out[1] = (acc >> 16) * 1;
}


/*
 * End
 */
