
#include <string.h>

#include "board.h"
#include "printd.h"
#include "can.h"
#include "led.h"
#include "cmd.h"
#include "ringbuffer.h"

#define CCAN_RX_MSG_ID 1
#define CCAN_TX_MSG_ID 23
#define CCAN_TX_MSG_REMOTE_ID 24

#define RB_SIZE 512

static uint32_t my_id;
static struct ringbuffer rx_rb;
static uint8_t rx_rb_buf[RB_SIZE];

static const PINMUX_GRP_T mux[] = {
        { 3,  1, (SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN | SCU_MODE_FUNC2) }, /* RD */
        { 3,  2, (SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN | SCU_MODE_FUNC2) }, /* TD */
        { 6, 10, (SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN | SCU_MODE_FUNC0) }, /* RS GPIO3[6] */
};


void can_init(uint32_t can_id)
{
	my_id = can_id;

	rb_init(&rx_rb, rx_rb_buf, sizeof(rx_rb_buf));

        Chip_SCU_SetPinMuxing(mux, sizeof(mux) / sizeof(mux[0]));

	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 3, 6);
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, 3, 6, 0);

	Chip_Clock_SetBaseClock(CLK_BASE_APB3, CLKIN_IDIVC, true, false);
	Chip_CCAN_Init(LPC_C_CAN0);
	Chip_CCAN_SetBitRate(LPC_C_CAN0, 250000);
	Chip_CCAN_EnableInt(LPC_C_CAN0, (CCAN_CTRL_IE | CCAN_CTRL_SIE | CCAN_CTRL_EIE));
	
	Chip_CCAN_AddReceiveID(LPC_C_CAN0, CCAN_MSG_IF1, 0);
	Chip_CCAN_AddReceiveID(LPC_C_CAN0, CCAN_MSG_IF1, can_id);
	NVIC_EnableIRQ(C_CAN0_IRQn);
}


void can_tx(uint16_t id, void *data, size_t len)
{
	CCAN_MSG_OBJ_T send_obj;

	if(len > 8) {
		return;
	}

	send_obj.id = id;
	send_obj.dlc = len;
	memcpy(send_obj.data, data, len);
	Chip_CCAN_Send(LPC_C_CAN0, CCAN_MSG_IF1, false, &send_obj);
	Chip_CCAN_ClearStatus(LPC_C_CAN0, CCAN_STAT_TXOK);
}


void CAN0_IRQHandler(void)
{
	CCAN_MSG_OBJ_T msg_buf;
	uint32_t can_int, can_stat, i;
	while ( (can_int = Chip_CCAN_GetIntID(LPC_C_CAN0)) != 0 ) {
		if (can_int & CCAN_INT_STATUS) {
			can_stat = Chip_CCAN_GetStatus(LPC_C_CAN0);
			// TODO with error or TXOK, RXOK
			if (can_stat & CCAN_STAT_EPASS) {
				printd("Passive error\r\n");
				return;
			}
			if (can_stat & CCAN_STAT_EWARN) {
				printd("Warning!!!\r\n");
				return;
			}
			if (can_stat & CCAN_STAT_BOFF) {
				printd("CAN bus is off\r\n");
				return;
			}
			Chip_CCAN_ClearStatus(LPC_C_CAN0, CCAN_STAT_TXOK);
			Chip_CCAN_ClearStatus(LPC_C_CAN0, CCAN_STAT_RXOK);
		}

		else if ((1 <= CCAN_INT_MSG_NUM(can_int)) && (CCAN_INT_MSG_NUM(can_int) <= 0x20)) {
		
			Chip_CCAN_GetMsgObject(LPC_C_CAN0, CCAN_MSG_IF1, can_int, &msg_buf);

			if(msg_buf.id == my_id) {
				for (i = 0; i < msg_buf.dlc; i++) {
					rb_push(&rx_rb, msg_buf.data[i]);
				}
				break;
			}
		}
	}
}


static uint8_t tx_buf[8];
static size_t tx_len = 0;

void can_tick(void)
{
	static int n = 0;
	if(n++ < 10000) return;
	n = 0;

	if(tx_len > 0) {
		can_tx(my_id + 0x100, tx_buf, tx_len);
		tx_len = 0;
	}
}


void can_uart_tx(uint8_t c)
{

	if(tx_len < 8) {
		tx_buf[tx_len++] = c;
	}

	if(tx_len == 8 || c == '\n') {
		can_tx(my_id + 0x100, tx_buf, tx_len);
		tx_len = 0;
	}
}


int can_uart_rx(uint8_t *c)
{
	return rb_pop(&rx_rb, c);
}


static int on_cmd_can(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	return 1;
}

CMD_REGISTER(can, on_cmd_can, "l[ist] | s[et] <id> <0|1>");

/*
 * End
 */

