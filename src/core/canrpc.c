
#include <string.h>
#include <math.h>

#include "chip.h"

#include "watchdog.h"
#include "romtab.h"
#include "evq.h"
#include "flash.h"
#include "can.h"
#include "printd.h"
#include "cmd.h"
#include "led.h"
#include "shared.h"

extern uint32_t mod_id; /* main.c */
extern struct cmd_cli cli; /*main */

static uint32_t flash_load_addr = 0;
static uint32_t mem_load_addr = 0;


static int on_can_set_led(uint8_t *data, size_t len)
{
	led_set(LED_ID_RED, data[0] ? LED_STATE_ON : LED_STATE_OFF);
	return 1;
}


static int on_can_reboot(uint8_t *data, size_t len)
{
	watchdog_reboot();
	return 1;
}


static int on_can_flash_set_load_addr(uint8_t *data, size_t len)
{
	flash_load_addr = (data[0] << 24) | (data[1] << 16) | 
		          (data[2] <<  8) | (data[3] <<  0);

	return 1;
}


static int on_can_flash_load_data(uint8_t *data, size_t len)
{
	flash_load(flash_load_addr, data, len);
	flash_load_addr += len;
	return 1;
}


static int on_can_flash_write(uint8_t *data, size_t len)
{
	uint32_t addr = (data[0] << 24) | (data[1] << 16) | 
		        (data[2] <<  8) | (data[3] <<  0);
	printd(".");
	flash_write_buf1(addr);
	return 1;
}


static int on_can_mem_set_load_addr(uint8_t *data, size_t len)
{
	mem_load_addr = (data[0] << 24) | (data[1] << 16) | 
		        (data[2] <<  8) | (data[3] <<  0);

	return 1;
}


static int on_can_mem_load_data(uint8_t *data, size_t len)
{
	memcpy((void *)mem_load_addr, data, len);
	mem_load_addr += len;
	return 1;
}


static int on_can_m4_cmd(uint8_t *data, size_t len)
{
	uint8_t cmd = data[0];

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
	
	return 0;
}


static uint8_t cli_rx_buf[8];
static size_t cli_rx_len = 0;
static uint8_t cli_tx_buf[8];
static size_t cli_tx_len = 0;

static void cli_tx_flush(void)
{
	if(cli_tx_len > 0) {
		can_tx(0x100 + mod_id, cli_tx_buf, cli_tx_len);
		cli_tx_len = 0;
	}
}


static void on_ev_tick_10hz(event_t *ev, void *data)
{
	cli_tx_flush();
}

EVQ_REGISTER(EV_TICK_10HZ, on_ev_tick_10hz);


static void cli_can_tx(uint8_t c)
{
	cli_tx_buf[cli_tx_len++] = c;
	if(c == '\n' || c == '\r' || cli_tx_len == 8) {
		cli_tx_flush();
	}
}


static int on_can_cli(uint8_t *data, size_t len)
{
	cli.tx = cli_can_tx;
	cli_rx_len = len;
	memcpy(cli_rx_buf, data, len);
	return 1;
}


static struct can_cmd {
	uint8_t cmd;
	uint8_t nargs;
	int (*fn)(uint8_t *data, size_t len);
} can_cmd_list[] = {
	{ 0x01, 1, on_can_set_led },
	{ 0x02, 0, on_can_reboot },
	{ 0x03, 4, on_can_flash_set_load_addr },
	{ 0x04, 0, on_can_flash_load_data },
	{ 0x05, 4, on_can_flash_write },
	{ 0x06, 4, on_can_mem_set_load_addr },
	{ 0x07, 0, on_can_mem_load_data },
	{ 0x08, 1, on_can_m4_cmd },
	{ 0x09, 0, on_can_cli },
	{ 0, 0, NULL },
};


static void on_ev_can(event_t *ev, void *data)
{
	struct ev_can *evc = &ev->can;

	if(0) {
		size_t i;
		printd("%03d %d [%d]  ", evc->id, evc->len);
		for(i=0; i<evc->len; i++) {
			printd(" %02x", evc->data[i]);
		}
		printd("\n");
	}

	if(evc->len >= 1) {
		uint8_t cmd = evc->data[0];

		struct can_cmd *cc;
		for(cc=can_cmd_list; cc->fn; cc++) {
			size_t nargs = evc->len - 1;
			if(cc->cmd == cmd && nargs >= cc->nargs) {
				uint8_t rsp[2];
				rsp[0] = cmd;
				rsp[1] = cc->fn(evc->data+1, nargs);
				can_tx(0x100 + mod_id, &rsp, sizeof(rsp));
			}
		}
	}

	if(cli_rx_len) {
		size_t i;
		for(i=0; i<cli_rx_len; i++) {
			cmd_cli_handle_char(&cli, cli_rx_buf[i]);
		}
		cli_rx_len = 0;
	}
}

EVQ_REGISTER(EV_CAN, on_ev_can);


