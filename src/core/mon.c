
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "chip.h"

#include "adc.h"
#include "cdc_uart.h"
#include "cmd.h"
#include "flash.h"
#include "i2s.h"
#include "led.h"
#include "uart.h"
#include "uda1380.h"
#include "watchdog.h"
#include "ssm2604.h"
#include "button.h"
#include "shared.h"
#include "mod.h"
#include "ifft.h"
#include "dpy.h"


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


#define GREEN "\e[32m"
#define YELLOW "\e[33;1m"
#define RED "\e[31;1m"

void mon_init(void)
{
	shared->scope.src = &shared->in[0];
}

struct cmd_cli *mon_cli = NULL;

static void out(const char *fmt, ...)
{
	if(mon_cli) {
		va_list va;
		va_start(va, fmt);
		cmd_vprintd(mon_cli, fmt, va);
		va_end(va);
	}
}


void mon_tick(void)
{
	if(mon_cli == NULL) return;
	static int n = 0;
	if(n++ < 100) return;
	n = 0;

	/* Channel levels */

	out("\e[H");
	out("DSP load: %3d.%d\e[K\n", shared->m4_load/10, shared->m4_load % 10);
	out("\e[K\n");
	shared->m4_load = 0;
	
	int i, j;
	for(i=0; i<16; i++) {
		uint32_t amp = (shared->level[i].max - shared->level[i].min);
		
		float dB = -96.33;
		if(amp > 1) {
			dB += 20 * logf(amp) / logf(10);
		}
		
		out("%s |", label[i]);
		out(GREEN);

		for(j=-70; j<0; j+=2) {
			if(j == -12) out(YELLOW);
			if(j ==  -6) out(RED);
			out("%s", dB >= j ? "■" : " ");
		}
		out("\e[0m|");
		float v1 = shared->level[i].min >> 11;
		float v2 = shared->level[i].max >> 11;
		int j;
		for(j=-16; j<=16; j++) {
			out("%s", (j >= v1 && j <= v2) ? "\e[33;1m◆\e[0m" : 
					(j == 0) ? "│" : "┄");
		}
		out("\e[0m| %+5.1f dB\e[K\n", dB);
		shared->level[i].min = INT32_MAX;
		shared->level[i].max = INT32_MIN;
	}

	dpy_clear();
	int x, y;
	for(y=4; y<64; y+=8) {
		for(x=4; x<128; x+=8) {
			dpy_pset(x, y, 1);
		}
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
			re[i] = n > 0 ? 8 * logf(n) : 0;
		}

		static const char *sym[] = { " ", "▂", "▃", "▄", "▅", "▆", "▇", "█" };
		int y, x;
		for(y=10; y>=0; y--) {
			out("│");
			if(y > 8) out(RED);
			else if(y > 6) out(YELLOW);
			else out(GREEN);
			for(x=0; x<SHARED_SCOPE_SIZE/2; x++) {
				int d = re[x] - y*8;
				if(d < 0) d = 0;
				if(d > 7) d = 7;
				out("%s", sym[d]);
			}
			out("\e[0m│\e[0K\n");
		}
		shared->scope.n = 0;

		for(x=0; x<SHARED_SCOPE_SIZE/2; x++) {
			int y = 64 - re[x];
			if(y < 0) y = 0;
			if(y > 95) y = 95;
			dpy_line(x*2+0, y, x*2+0, 96, 1);
		}
	}

	dpy_flush();
	
	out("\e[0J");
}


static int on_cmd_mon(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	if(argc >= 1u) {
		mon_cli = cli;
		char cmd = argv[0][0];

		if(cmd == 's') {
			if(argc >= 2u) {
				int idx = atoi(argv[1]);
				if(idx >= 1 && idx <= 8) {
					shared->scope.src = &shared->in[idx-1];
				} else if(idx >= 9 && idx <= 12) {
					shared->scope.src = &shared->out[idx-9];
				} 
			} else {
				shared->scope.src = NULL;
			}
		}
		cmd_printd(cli, "mon is now on %p\n", mon_cli);
	} else {
		mon_cli = NULL;
	}
	return 1;
}

CMD_REGISTER(mon, on_cmd_mon, "");

/*
 * End
 */
