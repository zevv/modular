
#include <string.h>

#include "chip.h"
#include "led.h"
#include "uart.h"

const uint32_t OscRateIn = 12000000;
const uint32_t ExtRateIn = 0;

static const uint8_t core[] = {
#include "core.inc"
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


void M0APP_IRQHandler(void)
{
	Chip_CREG_ClearM0AppEvent();
}


void main(void)
{
	arch_init();
	led_init();
	uart_init();

	uart_tx('1');

	led_set(LED_ID_RED, true);

	/* Copy core image for M0 to RAM2 bank */

	uart_tx('2');
	uint32_t core_base = 0x10080000;
	memcpy((void *)core_base, core, sizeof(core));

	/* Boot M0 */

	uart_tx('3');
	Chip_RGU_TriggerReset(RGU_M0APP_RST);
	Chip_Clock_Enable(CLK_M4_M0APP);
	Chip_CREG_SetM0AppMemMap(core_base);
	Chip_RGU_ClearReset(RGU_M0APP_RST);

	/* Wait for signal from M0 */
	
        NVIC_EnableIRQ(M0APP_IRQn);

	int n = 0;
	for(;;) {
		__WFI();
		led_set(LED_ID_BLUE, n);
		n = !n;
	}

	for(;;);
}


/*
 * End
 */
