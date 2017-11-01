
#include <string.h>
#include <math.h>

#include "chip.h"

#include "adc.h"
#include "cdc_uart.h"
#include "cmd.h"
#include "flash.h"
#include "i2s.h"
#include "led.h"
#include "printd.h"
#include "uart.h"
#include "uda1380.h"
#include "watchdog.h"
#include "ssm2604.h"
#include "button.h"
#include "shared.h"
#include "mod.h"
#include "ifft.h"

static bool mon_enable = false;

static const char *label[] = {
	"in   1", "in   2", "in   3", "in   4",
	"out  9", "out 10", "out 11", "out 12",
	"ctl  5", "ctl  6", "ctl  7", "ctl  8",
	"ctl  9", "ctl 10", "ctl 11", "ctl 12",
};

static const uint8_t hamming_window[SHARED_SCOPE_SIZE] = {
	19, 19, 20, 20, 21, 23, 24, 26, 28, 31, 33, 36, 39, 43, 46, 50, 54, 59,
	63, 68, 72, 77, 82, 88, 93, 99, 104, 110, 116, 121, 127, 133, 139, 145,
	150, 156, 162, 168, 173, 179, 184, 190, 195, 200, 205, 209, 214, 218,
	222, 226, 230, 234, 237, 240, 243, 245, 247, 249, 251, 253, 254, 255,
	255, 255, 255, 255, 255, 254, 253, 251, 249, 247, 245, 243, 240, 237,
	234, 230, 226, 222, 218, 214, 209, 205, 200, 195, 190, 184, 179, 173,
	168, 162, 156, 150, 145, 139, 133, 127, 121, 116, 110, 104, 99, 93, 88,
	82, 77, 72, 68, 63, 59, 54, 50, 46, 43, 39, 36, 33, 31, 28, 26, 24, 23,
	21, 20, 20, 19, 19,
};


void mon_tick(void)
{
	if(!mon_enable) return;
	static int n = 0;
	if(n++ < 100) return;
	n = 0;

	/* Channel levels */

	printd("\e[H");
	printd("DSP load: %3d.%d\e[K\n", shared->m4_load/10, shared->m4_load % 10);
	printd("\e[K\n");
	shared->m4_load = 0;
	
	int i, j;
	for(i=0; i<16; i++) {
		uint32_t amp = (shared->level[i].max - shared->level[i].min);
		
		float dB = -96.33;
		if(amp > 1) {
			dB += 20 * logf(amp) / logf(10);
		}
		
		printd("%s |", label[i]);

		for(j=-70; j<0; j+=2) {
			if(j == -12) printd("\e[32;1m");
			if(j == -12) printd("\e[33;1m");
			if(j ==  -6) printd("\e[31;1m");
			printd("%s", dB >= j ? "■" : " ");
		}
		printd("\e[0m|");
		float v1 = shared->level[i].min >> 11;
		float v2 = shared->level[i].max >> 11;
		int j;
		for(j=-16; j<=16; j++) {
			printd("%s", (j >= v1 && j <= v2) ? "\e[33;1m◆\e[0m" : 
					(j == 0) ? "|" : "┄");
		}
		printd("\e[0m| %+5.1f dB\e[K\n", dB);
		shared->level[i].min = INT32_MAX;
		shared->level[i].max = INT32_MIN;
	}

	/* Spectral analyzer */

	if(shared->scope.n == SHARED_SCOPE_SIZE) {
		int32_t re[SHARED_SCOPE_SIZE], im[SHARED_SCOPE_SIZE];
		int i;
		for(i=0; i<SHARED_SCOPE_SIZE; i++) {
			re[i] = shared->scope.buf[i] * hamming_window[i] >> 8;
			im[i] = 0;
		}
		ifft(re, im, SHARED_SCOPE_SIZE);
		for(i=0; i<SHARED_SCOPE_SIZE; i++) {
			/* Not accurate but fast */
			int32_t n = (abs(re[i])+abs(im[i])) >> 4;
			re[i] = 8 * logf(n);
		}

		int y, x;
		for(y=96; y>=16; y-=8) {
			for(x=0; x<SHARED_SCOPE_SIZE/2; x++) {
				printd("%s", re[x] > y ? "▊" : " ");
			}
			printd("\e[0m\e[0K\n");
		}
		shared->scope.n = 0;
	}
	
	printd("%d\e[0J");
}


static int on_cmd_mon(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	mon_enable = !mon_enable;
	printd("\e[2J");
	return 1;
}

CMD_REGISTER(mon, on_cmd_mon, "");

/*
 * End
 */
