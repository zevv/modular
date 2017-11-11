
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
#include "usb.h"
#include "ringbuffer.h"
#include "shared.h"
#include "evq.h"


static struct {
	volatile bool tx_busy;
	volatile bool rx_busy;
} midi;

static struct {
	volatile bool tx_busy;
	volatile bool rx_busy;
	struct ringbuffer tx_rb;
	uint8_t tx_rb_buf[256];
} clipipe;

/* 1192 bytes is returned by usbd_api->hw->GetMemSize() with the current
 * configuration */

static uint8_t usb_mem[1192] __attribute__((aligned(4096)));
static const USBD_API_T *usbd_api;
static USBD_HANDLE_T usbd;
static uint8_t my_id;
static uint8_t rx_buf[64];
static uint8_t tx_buf[64];


extern const uint8_t USB_DeviceDescriptor[];
extern uint8_t USB_FsConfigDescriptor[];
extern const uint8_t USB_StringDescriptor[];


static const PINMUX_GRP_T mux[] = {
        { 2, 11, (SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN | SCU_MODE_FUNC0)},
        { 2, 12, (SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN | SCU_MODE_FUNC0)},
	{ 2,  6, (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_FUNC4)},
	{ 2,  5, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC2)},
	{ 1,  7, (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_FUNC4)},
};



static void parse_midi(uint8_t *p, int n)
{
	while(n > 0) {
		printd("midi %02x %02x %02x %02x\n", p[0], p[1], p[2], p[3]);
		p += 4; n -= 4;
	}
}


static ErrorCode_t midi_bulk_handler(USBD_HANDLE_T usbd, void *ptr, uint32_t event)
{
	int n;

	switch(event) {

		case USB_EVT_IN:
			midi.tx_busy = false;
			break;

		case USB_EVT_OUT_NAK:
			if(!midi.rx_busy) {
				midi.rx_busy = true;
				usbd_api->hw->ReadReqEP(usbd, 0x01, (void *)rx_buf, sizeof(rx_buf));
			}
			break;
		
		case USB_EVT_OUT:
			n = usbd_api->hw->ReadEP(usbd, 0x01, (void *)rx_buf);
			parse_midi(rx_buf, n);
			midi.rx_busy = false;
			break;

		default:
			printd("ev %d\n", event);
			break;

	}

	return LPC_OK;
}


static void kick_tx(void)
{
	if(!USB_IsConfigured(usbd)) return;
	if(clipipe.tx_busy) return;

	size_t n = rb_used(&clipipe.tx_rb);

	if(n > 0) {
		if(n > sizeof(tx_buf)) n = sizeof(tx_buf);
		size_t i;
		for(i=0; i<n ;i++) {
			rb_pop(&clipipe.tx_rb, tx_buf + i);
		}
		clipipe.tx_busy = true;
		usbd_api->hw->WriteEP(usbd, 0x82, tx_buf, n);
	}
}


static ErrorCode_t cli_bulk_handler(USBD_HANDLE_T usbd, void *ptr, uint32_t event)
{
	size_t i = 0;
	size_t n;

	switch(event) {

		case USB_EVT_IN:
			clipipe.tx_busy = false;
			kick_tx();
			break;

		case USB_EVT_OUT_NAK:
			if(!clipipe.rx_busy && evq_room() >= sizeof(rx_buf)) {
				clipipe.rx_busy = true;
				usbd_api->hw->ReadReqEP(usbd, 0x02, (void *)rx_buf, sizeof(rx_buf));
			}
			break;

		case USB_EVT_OUT:
			n = usbd_api->hw->ReadEP(usbd, 0x02, (void *)rx_buf);
			while(n > 0) {
				event_t ev;
				size_t m = n < sizeof(ev.usbcli.data) ? n : sizeof(ev.usbcli.data);
				ev.type = EV_USBCLI;
				ev.usbcli.len = m;
				memcpy(ev.usbcli.data, rx_buf+i, m);
				evq_push(&ev);
				i += m;
				n -= m;
			}
			clipipe.rx_busy = false;
			break;

		default:
			printd("ev %d\n", event);
			break;

	}

	return LPC_OK;
}


void USB1_IRQHandler(void)
{
	usbd_api->hw->ISR(usbd);
}


static ErrorCode_t on_configure(USBD_HANDLE_T hUsb)
{
	usbd_api->hw->ReadReqEP(usbd, 0x01, (void *)rx_buf, sizeof(rx_buf));
	usbd_api->hw->ReadReqEP(usbd, 0x02, (void *)rx_buf, sizeof(rx_buf));
	midi.rx_busy = true;
	clipipe.rx_busy = true;
	return LPC_OK;
}



int usb_init(uint8_t id)
{
	USBD_API_INIT_PARAM_T param;
	USB_CORE_DESCS_T desc;
	ErrorCode_t ret = LPC_OK;
	uint32_t ep_indx;

	my_id = id;

        Chip_SCU_SetPinMuxing(mux, sizeof(mux) / sizeof(mux[0]));
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 5, 6);	  /* GPIO5[6] = USB1_PWR_EN */
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, 5, 6, true); /* GPIO5[6] output high */

	rb_init(&clipipe.tx_rb, clipipe.tx_rb_buf, sizeof(clipipe.tx_rb_buf));

	Chip_USB1_Init();

	usbd_api = (const USBD_API_T *) LPC_ROM_API->usbdApiBase;

	memset(&param, 0, sizeof(param));
	param.usb_reg_base = LPC_USB1_BASE;
	param.max_num_ep = 4;
	param.mem_base = (uint32_t)usb_mem;
	param.mem_size = sizeof(usb_mem);
	param.USB_Configure_Event = on_configure;

	desc.device_desc = (uint8_t *)USB_DeviceDescriptor;
	desc.string_desc = (uint8_t *)USB_StringDescriptor;
	desc.high_speed_desc = USB_FsConfigDescriptor;
	desc.full_speed_desc = USB_FsConfigDescriptor;

	assert(sizeof(usb_mem) == usbd_api->hw->GetMemSize(&param));

	ret = usbd_api->hw->Init(&usbd, &desc, &param);

	if (ret == LPC_OK) {
		ep_indx = (((0x01 & 0x0F) << 1));
		ret = usbd_api->core->RegisterEpHandler(usbd, ep_indx, midi_bulk_handler, &midi);
	}

	if (ret == LPC_OK) {
		ep_indx = (((0x81 & 0x0F) << 1) + 1);
		ret = usbd_api->core->RegisterEpHandler(usbd, ep_indx, midi_bulk_handler, &midi);
	}

	if (ret == LPC_OK) {
		ep_indx = (((0x02 & 0x0F) << 1));
		ret = usbd_api->core->RegisterEpHandler(usbd, ep_indx, cli_bulk_handler, &midi);
	}

	if (ret == LPC_OK) {
		ep_indx = (((0x82 & 0x0F) << 1) + 1);
		ret = usbd_api->core->RegisterEpHandler(usbd, ep_indx, cli_bulk_handler, &midi);
	}

	if(ret == LPC_OK) {

		NVIC_SetPriority(USB1_IRQn, 1);
		NVIC_EnableIRQ(USB1_IRQn);
		usbd_api->hw->Connect(usbd, 1);
	}

	return ret;
}


static void send_controls(void)
{
	size_t i;

	if(midi.tx_busy) return;

	for(i=0; i<12; i++) {
		if(shared->ctl[i].dirty) {
			shared->ctl[i].dirty = false;
			tx_buf[0] = 0x0b;
			tx_buf[1] = 0xb0 | 0;
			tx_buf[2] = 0x20 | i;
			tx_buf[3] = shared->ctl[i].val / 2;
			midi.tx_busy = true;
			usbd_api->hw->WriteEP(usbd, 0x81, tx_buf, 4);
			return;
		}
	}
}


void usb_tick(void)
{
	if(!USB_IsConfigured(usbd)) return;
	send_controls();
}


void usb_clipipe_tx(uint8_t c)
{
	rb_push(&clipipe.tx_rb, c);
	kick_tx();
}


int usb_clipipe_rx(uint8_t *c)
{
	return 0;
}



/*
 * End
 */
