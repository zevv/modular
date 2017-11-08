
/*
 * http://docs.lpcware.com/usbromlib/v1.0/index.html
 */

#include <string.h>
#include <math.h>
#include "board.h"
#include "led.h"
#include "cmd.h"
#include "uart.h"
#include "app_usbd_cfg.h"
#include "printd.h"
#include "midi.h"
#include "ringbuffer.h"
#include "shared.h"


#define CU_RXBUF_SZ	USB_HS_MAX_BULK_PACKET
#define CU_TXBUF_SZ	USB_HS_MAX_BULK_PACKET
#define RB_SIZE		1024

struct midi {
	USBD_HANDLE_T cdc;
	volatile bool tx_busy;
	volatile bool rx_busy;
	uint8_t *rx_buf;
	uint8_t *tx_buf;
	struct ringbuffer tx_rb;
	struct ringbuffer rx_rb;
	uint8_t rx_rb_buf[RB_SIZE];
	uint8_t tx_rb_buf[RB_SIZE];
};

static uint8_t usb_mem[USB_STACK_MEM_SIZE] __attribute__((aligned(4096)));
static const USBD_API_T *usbd_api;
static USBD_HANDLE_T usbd;
static struct midi midi;
static uint8_t my_id;
static uint8_t rx_buf[64];
static uint8_t tx_buf[64];

static const PINMUX_GRP_T mux[] = {
        { 2, 11, (SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN | SCU_MODE_FUNC0)},
        { 2, 12, (SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN | SCU_MODE_FUNC0)},
	{ 2,  6, (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_FUNC4)},
	{ 2,  5, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC2)},
	{ 1,  7, (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_FUNC4)},
};


void handle_sysex(uint8_t c)
{
	if(!(c & 0x80)) {
		rb_push(&midi.rx_rb, c);
		//uart_tx(c);
	}
}


static void parse_midi(uint8_t *p, int n)
{
	while(n > 0) {
		switch(p[0]) {
			case 0x04: /* sysex start or cont */
				handle_sysex(p[1]);
				handle_sysex(p[2]);
				handle_sysex(p[3]);
				break;
			case 0x05: /* sysex last one */
				handle_sysex(p[1]);
				break;
			case 0x06: /* sysex last two */
				handle_sysex(p[1]);
				handle_sysex(p[2]);
				break;
			case 0x07: /* sysex last three */
				handle_sysex(p[1]);
				handle_sysex(p[2]);
				handle_sysex(p[3]);
				break;
			default:
				printd("midi %02x %02x %02x %02x\n", p[0], p[1], p[2], p[3]);
				break;
		}

		p += 4; n -= 4;
	}
}

static void kick_tx(void)
{
	if(!USB_IsConfigured(usbd)) return;
	if(midi.tx_busy) return;

	size_t n = rb_used(&midi.tx_rb) & 0xfffc;
	if(n > 0) {
		if(n > 4) n = 4;
		size_t i;
		for(i=0; i<n; i++) {
			uint8_t c ;
			rb_pop(&midi.tx_rb, &c);
			tx_buf[i] = c;
		}
		midi.tx_busy = true;
		usbd_api->hw->WriteEP(usbd, 0x81, tx_buf, n);
	}
}


static ErrorCode_t midi_bulk_handler(USBD_HANDLE_T usbd, void *ptr, uint32_t event)
{
	int n;

	switch(event) {

		case USB_EVT_IN:
			midi.tx_busy = false;
			kick_tx();
			break;

		case USB_EVT_OUT_NAK:
			if(!midi.rx_busy && rb_free(&midi.rx_rb) >= CU_RXBUF_SZ) {
				midi.rx_busy = true;
				usbd_api->hw->ReadReqEP(usbd, 0x01, (void *)rx_buf, sizeof(rx_buf));
			}
			break;
		
		case USB_EVT_OUT:
			n = usbd_api->hw->ReadEP(usbd, USB_CDC_OUT_EP, (void *)rx_buf);
			parse_midi(rx_buf, n);
			midi.rx_busy = false;
			break;

		default:
			printd("ev %d\n", event);
			break;

	}

	return LPC_OK;
}


void USB_IRQHandler(void)
{
	usbd_api->hw->ISR(usbd);
}


static ErrorCode_t on_configure(USBD_HANDLE_T hUsb)
{
	printd("Conf\n");
	usbd_api->hw->ReadReqEP(usbd, 0x01, (void *)rx_buf, sizeof(rx_buf));
	return LPC_OK;
}



int midi_init(uint8_t id)
{
	USBD_API_INIT_PARAM_T param;
	USB_CORE_DESCS_T desc;
	ErrorCode_t ret = LPC_OK;
	uint32_t ep_indx;
	USB_CDC_CTRL_T *pCDC;

	my_id = id;

        Chip_SCU_SetPinMuxing(mux, sizeof(mux) / sizeof(mux[0]));
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 5, 6);	  /* GPIO5[6] = USB1_PWR_EN */
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, 5, 6, true); /* GPIO5[6] output high */
	
	USB_init_pin_clk();

	usbd_api = (const USBD_API_T *) LPC_ROM_API->usbdApiBase;

	memset(&param, 0, sizeof(param));
	param.usb_reg_base = LPC_USB_BASE;
	param.max_num_ep = 2;
	param.mem_base = (uint32_t)usb_mem;
	param.mem_size = USB_STACK_MEM_SIZE;
	param.USB_Configure_Event = on_configure;

	desc.device_desc = (uint8_t *)USB_DeviceDescriptor;
	desc.string_desc = (uint8_t *)USB_StringDescriptor;
	desc.high_speed_desc = USB_FsConfigDescriptor;
	desc.full_speed_desc = USB_FsConfigDescriptor;

	ret = usbd_api->hw->Init(&usbd, &desc, &param);

	if (ret == LPC_OK) {
		ep_indx = (((0x01 & 0x0F) << 1));
		ret = usbd_api->core->RegisterEpHandler(usbd, ep_indx, midi_bulk_handler, &midi);
	}
	
	if (ret == LPC_OK) {
		ep_indx = (((0x81 & 0x0F) << 1) + 1);
		ret = usbd_api->core->RegisterEpHandler(usbd, ep_indx, midi_bulk_handler, &midi);
	}

	if(ret == LPC_OK) {
		pCDC = (USB_CDC_CTRL_T *) midi.cdc;
		pCDC->line_coding.dwDTERate = 115200;
		pCDC->line_coding.bDataBits = 8;


		rb_init(&midi.tx_rb, midi.tx_rb_buf, sizeof(midi.tx_rb_buf));
		rb_init(&midi.rx_rb, midi.rx_rb_buf, sizeof(midi.rx_rb_buf));

		NVIC_SetPriority(LPC_USB_IRQ, 1);
		NVIC_EnableIRQ(LPC_USB_IRQ);
		usbd_api->hw->Connect(usbd, 1);
	}

	return ret;
}


static void send_controls(void)
{
	size_t i;
	bool work = false;

	for(i=0; i<12; i++) {
		if(shared->ctl[i].dirty) {
			shared->ctl[i].dirty = false;
			rb_push(&midi.tx_rb, 0x0b);
			rb_push(&midi.tx_rb, 0xb0 | 0);
			rb_push(&midi.tx_rb, 0x20 | i);
			rb_push(&midi.tx_rb, shared->ctl[i].val / 2);
			work ++;
		}
	}
	if(work) {
		kick_tx();
	}
}


void send_sysex(void)
{
	while(!midi.tx_busy) {
		int n = rb_used(&midi.tx_rb);
		if(n == 0) return;

		if(n > 40) n = 40;
		n += 2;
		int i = 0;
		int o = 0;

		while(i < n) {
			if(n > 3) {
				tx_buf[o++] = 0x04;
			} else if(n == 3) {
				tx_buf[o++] = 0x07;
			} else if(n == 2) {
				tx_buf[o++] = 0x06;
			} else if(n == 1) {
				tx_buf[o++] = 0x05;
			}

			int k = n <= 3 ? n : 3;

			while(k--) {
				if(i == 0) {
					tx_buf[o++] = 0xf0;
				} else if(i == n-1) {
					tx_buf[o++] = 0xf7;
				} else {
					rb_pop(&midi.tx_rb, tx_buf + o++);
				}
				i ++;
			}
		}

		midi.tx_busy = true;
		usbd_api->hw->WriteEP(usbd, 0x81, tx_buf, o);
	}
}


void midi_tick(void)
{
	if(!USB_IsConfigured(usbd)) return;
	if(0) send_controls();
	//send_sysex();
}


void midi_tx(uint8_t c)
{
	if(rb_free(&midi.tx_rb) > 8) {
		rb_push(&midi.tx_rb, 0x07);
		rb_push(&midi.tx_rb, 0xf0);
		rb_push(&midi.tx_rb, c);
		rb_push(&midi.tx_rb, 0xf7);
		kick_tx();
	} else {
		printd("x\n");
	}
}


int midi_rx(uint8_t *c)
{
	return rb_pop(&midi.rx_rb, c);
}


static int on_cmd_midi(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	uint8_t data[4];

	data[0] = 0x19;
	data[1] = 0x91;
	data[2] = 0x30;
	data[3] = 0x60;
	usbd_api->hw->WriteEP(usbd, 0x81, (void *)data, sizeof(*data));
	return 1;
}

CMD_REGISTER(midi, on_cmd_midi, "l[ist] | s[et] <id> <0|1>");


/*
 * End
 */
