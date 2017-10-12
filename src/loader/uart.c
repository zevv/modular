
#define USE_CDC_UART

#include "chip.h"
#include "uart.h"


STATIC const PINMUX_GRP_T mux[] = {
	{ 0x6, 4, (SCU_MODE_PULLDOWN | SCU_MODE_FUNC2) },
	{ 0x6, 5, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC2) },
};


void uart_init(void)
{
        Chip_SCU_SetPinMuxing(mux, sizeof(mux) / sizeof(mux[0]));

	SystemCoreClockUpdate();
	Chip_UART_SetBaud(LPC_USART0, 115200);
	Chip_UART_ConfigData(LPC_USART0, UART_LCR_WLEN8 | UART_LCR_SBS_1BIT);
	Chip_UART_TXEnable(LPC_USART0);

        Chip_UART_SetupFIFOS(LPC_USART0, (UART_FCR_FIFO_EN | UART_FCR_RX_RS | UART_FCR_TX_RS | UART_FCR_TRG_LEV3));
}


void uart_tx(uint8_t c)
{
	while(!(LPC_USART0->LSR & UART_LSR_THRE));
	LPC_USART0->THR = c;
}


/*
 * End
 */
