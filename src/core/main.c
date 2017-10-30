
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

static void mon_tick(void);
static bool mon_enable = false;

const uint32_t OscRateIn = 12000000;
const uint32_t ExtRateIn = 0;

static struct cmd_cli cli1 = {
	.rx = uart_rx,
	.tx = uart_tx,
	.echo = true,
};

static struct cmd_cli cli2 = {
	.rx = cdc_uart_rx,
	.tx = cdc_uart_tx,
	.echo = true,
};


void arch_init(void)
{
	Chip_SetupCoreClock(CLKIN_CRYSTAL, MAX_CLOCK_FREQ, true);
	Chip_Clock_SetBaseClock(CLK_BASE_USB1, CLKIN_IDIVD, true, true);

	/* We seem to need a delay for clocks to come up otherwise USB won't
	 * work? */

	volatile int i;
	for(i=0; i<100000; i++);
}


static void to_shared_log(uint8_t c)
{
	shared->log.buf[shared->log.head] = c;
	shared->log.head = (shared->log.head + 1) % SHARED_LOG_BUF_SIZE;
}


static void logd(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vfprintd(to_shared_log, fmt, va);
	va_end(va);
}


static void read_m4_log(void)
{
	if(shared->log.tail != shared->log.head) {
		printd("\e[36m");
		while(shared->log.tail != shared->log.head) {
			printd("%c", shared->log.buf[shared->log.tail]);
			shared->log.tail = (shared->log.tail + 1) % SHARED_LOG_BUF_SIZE;
		}
		printd("\e[0m");
	}
}



/*
 * This function is called by the M4; it is defined here to allow
 * RAM1 to be updated with new code while the M4 waits here until
 * finished. Blink the blue led like crazy until we can go on
 */

void main(void)
{
	arch_init();
	led_init();
	button_init();
	uart_init();
	//watchdog_init();
	printd_set_handler(uart_tx);

	printd("\n\nHello %s %s %s\n", VERSION, __DATE__, __TIME__);

	memset((void *)shared, 0, sizeof(*shared));

	cdc_uart_init();
	printd_set_handler(cdc_uart_tx);

	flash_init();
	adc_init();
	i2s_init(SRATE);
	ssm2604_init();

	shared->logd = logd;
	logd("M0 ready\n");

	//mod_load_name("bypass");

	int n = 0;
	static bool bp = false;
	static int mod_id = 0;

	for(;;) {
		cmd_cli_poll(&cli1);
		cmd_cli_poll(&cli2);
		read_m4_log();
		led_set(LED_ID_GREEN, (n++ & 0x200) ? LED_STATE_ON : LED_STATE_OFF);
		volatile int i;
		for(i=0; i<10000; i++);
		i2s_tick();
		adc_tick();
		watchdog_poll();
		mon_tick();

		bool b = button_get();
		if(b && !bp) {
			mod_load_id(mod_id);
			mod_id = (mod_id + 1) % 8;
		}
		bp = b;
	}
}


static int on_cmd_reboot(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	watchdog_reboot();
	return 1;
}


CMD_REGISTER(reboot, on_cmd_reboot, "");


static int on_cmd_version(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	cmd_printd(cli, "modular %s / %s / %s %s\n", VERSION, BUILD, __DATE__, __TIME__);
	return 1;
}

CMD_REGISTER(version, on_cmd_version, "");


static int on_cmd_m4(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	if(argc >= 1u) {
		char cmd = argv[0][0];

		if(cmd == 's') { /* stop */
			shared->m4_state = M4_STATE_FADEOUT;
			return 1;
		}

		if(cmd == 'g') { /* go */
			shared->m4_state = M4_STATE_FADEIN;
			Chip_RGU_TriggerReset(RGU_M3_RST);
			return 1;
		}

		if(cmd == 'h') { /* halt */
			shared->m4_state = M4_STATE_HALT;
			return 1;
		}

		if(cmd == 'l') {
			cmd_printd(cli, "%d.%d\n", shared->m4_load/10, shared->m4_load % 10);
			shared->m4_load = 0;
			return 1;
		}

		if(cmd == 'w') {
			for(;;);
		}

	}

	return 1;
}

CMD_REGISTER(m4, on_cmd_m4, "");

static int on_cmd_clock(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	if(argc == 1) {
		uint32_t rate = strtol(argv[0], NULL, 0) * 1000000;
		if(rate >= 8000000 && rate <= 204000000) {
			Chip_SetupCoreClock(CLKIN_CRYSTAL, rate, true);
		}
	}
	
	cmd_printd(cli, "%d\n", Chip_Clock_GetMainPLLHz());
	return 1;
}


CMD_REGISTER(clock, on_cmd_clock, "");

static const char *label[] = {
	"in   1", "in   2", "in   3", "in   4",
	"out  9", "out 10", "out 11", "out 12",
	"ctl  5", "ctl  6", "ctl  7", "ctl  8",
	"ctl  9", "ctl 10", "ctl 11", "ctl 12",
};

static void mon_tick(void)
{
	if(!mon_enable) return;
	static int n = 0;
	if(n++ < 100) return;
	n = 0;

	printd("\e[H");
	printd("DSP load: %3d.%d\n", shared->m4_load/10, shared->m4_load % 10);
	shared->m4_load = 0;

	printd("\n");
	
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
		printd("\e[0m| %+5.1f dB      \n", dB);
		shared->level[i].min = INT32_MAX;
		shared->level[i].max = INT32_MIN;
	}
}


static int on_cmd_mon(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	mon_enable = !mon_enable;
	printd("\e[2J");
	return 1;
}

CMD_REGISTER(mon, on_cmd_mon, "");


static int on_cmd_log(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	cmd_hexdump(cli, (void *)shared->log.buf, SHARED_LOG_BUF_SIZE, 0);
	return 1;
}

CMD_REGISTER(log, on_cmd_log, "");

/*
 * End
 */
