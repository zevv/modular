
#define USE_CDC_UART

#include "board.h"
#include "chip.h"

#include "uart.h"

#define UART_RRB_SIZE 128

STATIC RINGBUFF_T rxring;
static uint8_t rxbuff[UART_RRB_SIZE];


void uart_init(void)
{
	Board_UART_Init(LPC_USART0);
	SystemCoreClockUpdate();
	Chip_UART_SetBaud(LPC_USART0, 115200);
	Chip_UART_ConfigData(LPC_USART0, UART_LCR_WLEN8 | UART_LCR_SBS_1BIT);
	Chip_UART_TXEnable(LPC_USART0);

	RingBuffer_Init(&rxring, rxbuff, 1, UART_RRB_SIZE);
        Chip_UART_SetupFIFOS(LPC_USART0, (UART_FCR_FIFO_EN | UART_FCR_RX_RS | UART_FCR_TX_RS | UART_FCR_TRG_LEV3));
	Chip_UART_IntEnable(LPC_USART0, (UART_IER_RBRINT | UART_IER_RLSINT));
	NVIC_EnableIRQ(USART0_IRQn);


}


void uart_tx(uint8_t c)
{
	while(!(LPC_USART0->LSR & UART_LSR_THRE));
	LPC_USART0->THR = c;
}


int uart_rx(uint8_t *c)
{
	return Chip_UART_ReadRB(LPC_USART0, &rxring, c, 1);
}


void usart_irq(void)
{
	Chip_UART_RXIntHandlerRB(LPC_USART0, &rxring);
}

/*
 * End
 */
