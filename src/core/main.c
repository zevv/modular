
#include <string.h>
#include <math.h>

#include "chip.h"

#include "adc.h"
#include "mon.h"
#include "evq.h"
#include "can.h"
#include "usb.h"
#include "timer.h"
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
#include "dpy.h"

#define LPC_OTP ((LPC_OTP_T *) LPC_OTP_BASE)
	
uint32_t mod_id = 0;

const uint32_t OscRateIn = 12000000;
const uint32_t ExtRateIn = 0;

struct cmd_cli cli = {
	.tx = NULL,
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




/*
 * This function is called by the M4; it is defined here to allow
 * RAM1 to be updated with new code while the M4 waits here until
 * finished. Blink the blue led like crazy until we can go on
 */

void main(void)
{
	uint32_t id = LPC_OTP->OTP0_2;
	if(id == 0x24b14191) mod_id = 1;
	if(id == 0x1ab14191) mod_id = 2;
	if(id == 0x26c94191) mod_id = 3;

	memset((void *)shared, 0, sizeof(*shared));
	shared->logd = logd;

	arch_init();
	watchdog_init();
	led_init();
	button_init();
	uart_init();

	printd_set_handler(uart_tx);
	printd("\n\nHello %s %s %s %08x\n", VERSION, __DATE__, __TIME__);

	timer_init();
	usb_init(mod_id);

	flash_init();
	adc_init();
	i2s_init(SRATE);
	ssm2604_init();
	can_init(mod_id); /* Must be done after I2C/ADC setup, see errata 3.2 */
	dpy_init();
	mon_init();
		
	NVIC_EnableIRQ(M4_IRQn);

	logd("M0 ready %08x\n", id);

	if(mod_id == 1) mod_load_name("vco");
	if(mod_id == 2) mod_load_name("vcf");
	if(mod_id == 3) mod_load_name("reverb");

	for(;;) {
		event_t ev;
		evq_wait(&ev);
		watchdog_poll();
	}
}


void M4_IRQHandler(void)
{
	Chip_CREG_ClearM4Event();
	event_t ev;
	ev.type = EV_M4;
	evq_push(&ev);
}

static void on_ev_m4(event_t *ev, void *data)
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

	

EVQ_REGISTER(EV_M4, on_ev_m4);



static void on_ev_usbcli(event_t *ev, void *data)
{
	size_t i;
	cli.tx = usb_clipipe_tx;
	for(i=0; i<ev->usbcli.len; i++) {
		cmd_cli_handle_char(&cli, ev->usbcli.data[i]);
	}
}

EVQ_REGISTER(EV_USBCLI, on_ev_usbcli);


static void on_ev_uart(event_t *ev, void *data)
{
	cli.tx = uart_tx;
	uint8_t c = ev->uart.data;
	cmd_cli_handle_char(&cli, c);
}

EVQ_REGISTER(EV_UART, on_ev_uart);


static void on_ev_tick_10hz(event_t *ev, void *data)
{
	static int m = false;
	led_set(LED_ID_GREEN, m ? LED_STATE_ON : LED_STATE_OFF);
	m = !m;
}

EVQ_REGISTER(EV_TICK_10HZ, on_ev_tick_10hz);


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


static int on_cmd_log(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	cmd_hexdump(cli, (void *)shared->log.buf, SHARED_LOG_BUF_SIZE, 0);
	return 1;
}

CMD_REGISTER(log, on_cmd_log, "");

/*
 * End
 */
