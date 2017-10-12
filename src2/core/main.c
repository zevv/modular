
#include "chip.h"

#include "led.h"
#include "uart.h"
#include "cdc_uart.h"
#include "cmd.h"
#include "printd.h"
#include "flash.h"

const uint32_t OscRateIn = 12000000;
const uint32_t ExtRateIn = 0;

static struct cmd_cli cli1 = {
	.rx = uart_rx,
	.tx = uart_tx,
};

static struct cmd_cli cli2 = {
	.rx = cdc_uart_rx,
	.tx = cdc_uart_tx,
};

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



void main(void)
{
	arch_init();
	led_init();
	uart_init();
	cdc_uart_init();
	flash_init();
//	printd_set_handler(cdc_uart_tx);


	led_set(LED_ID_RED, true);

	int n =0;

	for(;;) {
		cmd_cli_poll(&cli1);
		cmd_cli_poll(&cli2);
		led_set(LED_ID_RED, n);
		n = !n;
		volatile int i;
		for(i=0; i<1000000; i++);
	}
}


static int on_cmd_switch(struct cmd_cli *cli, uint8_t argc, char **argv)
{
    __DSB();
    __SEV();
    cmd_printd(cli, "done!\n");
    return 1;
}


CMD_REGISTER(hop, on_cmd_switch, "");

/*
 * End
 */
