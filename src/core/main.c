
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


static void delay(int n)
{
	volatile int i;
	for(i=0; i<n; i++);
}


void audio_init(void)
{
	Chip_SCU_PinMuxSet(0x2, 10, (SCU_MODE_PULLUP | SCU_MODE_FUNC0));
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, 14);

	Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, 14, true);
	delay(1000000);
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, 14, false);
	delay(1000000);

	UDA1380_Init(0);
}


void SysTick_Handler(void)
{

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
	printd_set_handler(uart_tx);
	
	printd("\n\nHello %s %s %s\n", VERSION, __DATE__, __TIME__);

	memset((void *)shared, 0, sizeof(*shared));

	cdc_uart_init();
	flash_init();
	adc_init();
	i2s_init(48000);
	audio_init();
	SysTick_Config(SystemCoreClock / 1000);

	int n =0;

	for(;;) {
		cmd_cli_poll(&cli1);
		cmd_cli_poll(&cli2);
		led_set(LED_ID_RED, (n++ & 0x3f0) == 0x10);
		volatile int i;
		for(i=0; i<10000; i++);
		i2s_tick();
		calc_m4_load();
	}
}


static int on_cmd_reboot(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	watchdog_reboot();
	return 1;
}


CMD_REGISTER(reboot, on_cmd_reboot, "");



static int on_cmd_hop(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	if(argc >= 1u) {
		char cmd = argv[0][0];

		if(cmd == '0') {
			m4_active = false;
		}

		if(cmd == '1') {
			m4_active = true;
			Chip_RGU_TriggerReset(RGU_M3_RST);
		}

		if(cmd == 'l') {
			cmd_printd(cli, "%d %d\n", m4_load, shared->m4_ticks);
			return 1;
		}
	}

	return 1;
}


CMD_REGISTER(hop, on_cmd_hop, "");

/*
 * End
 */
