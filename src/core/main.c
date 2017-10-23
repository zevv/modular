
#include <string.h>

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
#include "shared.h"

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


bool m4_active = false;

void arch_init(void)
{
//	Chip_SetupCoreClock(CLKIN_IRC, MAX_CLOCK_FREQ, true);
	Chip_SetupCoreClock(CLKIN_CRYSTAL, MAX_CLOCK_FREQ, true);
	Chip_Clock_SetBaseClock(CLK_BASE_USB1, CLKIN_IDIVD, true, true);

	/* We seem to need a delay for clocks to come up otherwise USB won't
	 * work? */

	volatile int i;
	for(i=0; i<100000; i++);
}


static int m4_load;

static void calc_m4_load(void)
{
	static int n = 0;
	static int load = 0;

	if(n++ < 1000) {
		if(shared->m4_busy) load++;
	} else {
		m4_load = load;
		load = 0;
		n = 0;
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

	int n = 0;

	for(;;) {
		cmd_cli_poll(&cli1);
		cmd_cli_poll(&cli2);
		led_set(LED_ID_GREEN, (n++ & 0x200) ? LED_STATE_ON : LED_STATE_OFF);
		volatile int i;
		for(i=0; i<10000; i++);
		i2s_tick();
		adc_tick();
		calc_m4_load();
		watchdog_poll();
	}
}


static int on_cmd_reboot(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	watchdog_reboot();
	return 1;
}


CMD_REGISTER(reboot, on_cmd_reboot, "");



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
			m4_active = true;
			Chip_RGU_TriggerReset(RGU_M3_RST);
			return 1;
		}

		if(cmd == 'h') { /* halt */
			m4_active = false;
			return 1;
		}

		if(cmd == 'l') {
			cmd_printd(cli, "%d %d %d\n", m4_load, shared->m4_ticks, shared->m4_state);
			return 1;
		}

		if(cmd == 'w') {
			for(;;);
		}

	}

	return 1;
}


CMD_REGISTER(m4, on_cmd_m4, "");

/*
 * End
 */
