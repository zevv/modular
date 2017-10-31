
/*
 * http://docs.lpcware.com/usbromlib/v1.0/index.html
 */

#include <string.h>
#include "board.h"
#include "app_usbd_cfg.h"
#include "cdc_uart.h"
#include "ringbuffer.h"


#define CU_RXBUF_SZ	USB_HS_MAX_BULK_PACKET
#define CU_TXBUF_SZ	USB_HS_MAX_BULK_PACKET
#define RB_SIZE		1024

struct cdc_uart {
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

static const USBD_API_T *usbd_api;
static USBD_HANDLE_T usbd;
static struct cdc_uart cu;

static const PINMUX_GRP_T mux[] = {
	{ 2,  6, (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_FUNC4)},
	{ 2,  5, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC2)},
	{ 1,  7, (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_FUNC4)},
};


static void kick_tx(void)
{
	if(!USB_IsConfigured(usbd)) return;
	if(cu.tx_busy) return;

	size_t n = rb_used(&cu.tx_rb);

	if(n > 0) {
		size_t i;
		for(i=0; i<n ;i++) {
			rb_pop(&cu.tx_rb, cu.tx_buf + i);
		}
		cu.tx_busy = true;
		usbd_api->hw->WriteEP(usbd, USB_CDC_IN_EP, cu.tx_buf, n);
	}
}


static ErrorCode_t cdc_bulk_handler(USBD_HANDLE_T usbd, void *data, uint32_t event)
{
	int n, i;

	switch (event) {

		case USB_EVT_IN:
			cu.tx_busy = false;
			kick_tx();
			break;

		case USB_EVT_OUT_NAK:
			if(!cu.rx_busy && rb_free(&cu.rx_rb) >= CU_RXBUF_SZ) {
				cu.rx_busy = true;
				usbd_api->hw->ReadReqEP(usbd, USB_CDC_OUT_EP, cu.rx_buf, CU_RXBUF_SZ);
			}
			break;

		case USB_EVT_OUT:
			n = usbd_api->hw->ReadEP(usbd, USB_CDC_OUT_EP, cu.rx_buf);
			for(i=0; i<n; i++) {
				rb_push(&cu.rx_rb, cu.rx_buf[i]);
			}
			cu.rx_busy = false;
			break;

		default:
			break;
	}

	return LPC_OK;
}


void USB_IRQHandler(void)
{
	usbd_api->hw->ISR(usbd);
}


static USB_INTERFACE_DESCRIPTOR *find_IntfDesc(const uint8_t *pDesc, uint32_t intfClass)
{
	USB_COMMON_DESCRIPTOR *pD;
	USB_INTERFACE_DESCRIPTOR *pIntfDesc = 0;
	uint32_t next_desc_adr;

	pD = (USB_COMMON_DESCRIPTOR *) pDesc;
	next_desc_adr = (uint32_t) pDesc;

	while (pD->bLength) {
		if (pD->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE) {
			pIntfDesc = (USB_INTERFACE_DESCRIPTOR *) pD;
			if (pIntfDesc->bInterfaceClass == intfClass) {
				break;
			}
		}
		pIntfDesc = 0;
		next_desc_adr = (uint32_t) pD + pD->bLength;
		pD = (USB_COMMON_DESCRIPTOR *) next_desc_adr;
	}

	return pIntfDesc;
}


static ErrorCode_t on_configure(USBD_HANDLE_T hUsb)
{
	//kick_tx();
	return LPC_OK;
}


uint8_t usb_mem[USB_STACK_MEM_SIZE] __attribute__((aligned(4096)));

int cdc_uart_init(void)
{
	USBD_API_INIT_PARAM_T usb_param;
	USB_CORE_DESCS_T desc;
	ErrorCode_t ret = LPC_OK;
	USBD_CDC_INIT_PARAM_T cdc_param;
	uint32_t ep_indx;
	USB_CDC_CTRL_T *pCDC;

        Chip_SCU_SetPinMuxing(mux, sizeof(mux) / sizeof(mux[0]));
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 5, 6);	  /* GPIO5[6] = USB1_PWR_EN */
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, 5, 6, true); /* GPIO5[6] output high */


	usbd_api = (const USBD_API_T *) LPC_ROM_API->usbdApiBase;

	memset(&usb_param, 0, sizeof(usb_param));
	usb_param.usb_reg_base = LPC_USB_BASE;
	usb_param.max_num_ep = 4;
	usb_param.mem_base = (uint32_t)usb_mem;
	usb_param.mem_size = USB_STACK_MEM_SIZE;
	usb_param.USB_Configure_Event = on_configure;

	desc.device_desc = (uint8_t *)USB_DeviceDescriptor;
	desc.string_desc = (uint8_t *)USB_StringDescriptor;
	desc.high_speed_desc = USB_HsConfigDescriptor;
	desc.full_speed_desc = USB_FsConfigDescriptor;
	desc.device_qualifier = (uint8_t *)USB_DeviceQualifier;
	
	USB_init_pin_clk();

	ret = usbd_api->hw->Init(&usbd, &desc, &usb_param);
	
	/* Init CDC driver */

	if(ret == LPC_OK) {
		memset((void *) &cdc_param, 0, sizeof(USBD_CDC_INIT_PARAM_T));
		cdc_param.mem_base = usb_param.mem_base;
		cdc_param.mem_size = usb_param.mem_size;
		cdc_param.cif_intf_desc = (uint8_t *) find_IntfDesc(desc.high_speed_desc, CDC_COMMUNICATION_INTERFACE_CLASS);
		cdc_param.dif_intf_desc = (uint8_t *) find_IntfDesc(desc.high_speed_desc, CDC_DATA_INTERFACE_CLASS);

		ret = usbd_api->cdc->init(usbd, &cdc_param, &cu.cdc);
	}
		
	/* Set buffer pointers and register endpoint handler */
	
	if (ret == LPC_OK) {
		cu.tx_buf = (uint8_t *) cdc_param.mem_base;
		cdc_param.mem_base += CU_TXBUF_SZ;
		cdc_param.mem_size -= CU_TXBUF_SZ;
		cu.rx_buf = (uint8_t *) cdc_param.mem_base;
		cdc_param.mem_base += CU_RXBUF_SZ;
		cdc_param.mem_size -= CU_RXBUF_SZ;

		ep_indx = (((USB_CDC_IN_EP & 0x0F) << 1) + 1);
		ret = usbd_api->core->RegisterEpHandler(usbd, ep_indx, cdc_bulk_handler, &cu);
	}

	if (ret == LPC_OK) {
		ep_indx = ((USB_CDC_OUT_EP & 0x0F) << 1);
		ret = usbd_api->core->RegisterEpHandler(usbd, ep_indx, cdc_bulk_handler, &cu);
	}

	if(ret == LPC_OK) {
		pCDC = (USB_CDC_CTRL_T *) cu.cdc;
		pCDC->line_coding.dwDTERate = 115200;
		pCDC->line_coding.bDataBits = 8;

		usb_param.mem_base = cdc_param.mem_base;
		usb_param.mem_size = cdc_param.mem_size;

		rb_init(&cu.tx_rb, cu.tx_rb_buf, sizeof(cu.tx_rb_buf));
		rb_init(&cu.rx_rb, cu.rx_rb_buf, sizeof(cu.rx_rb_buf));

		NVIC_SetPriority(LPC_USB_IRQ, 1);
		NVIC_EnableIRQ(LPC_USB_IRQ);
		usbd_api->hw->Connect(usbd, 1);
	}

	return ret;
}


void cdc_uart_tx(uint8_t c)
{
	rb_push(&cu.tx_rb, c);
	kick_tx();
}


int cdc_uart_rx(uint8_t *c)
{
	return rb_pop(&cu.rx_rb, c);
}


/*
 * End
 */
