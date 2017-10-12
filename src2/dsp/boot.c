
#include "chip.h"

#define WEAK __attribute__ ((weak))
#define ALIAS(f) __attribute__ ((weak, alias (#f)))

void _boot(void);

WEAK void NMI_Handler(void);
WEAK void HardFault_Handler(void);
WEAK void MemManage_Handler(void);
WEAK void BusFault_Handler(void);
WEAK void UsageFault_Handler(void);
WEAK void SVC_Handler(void);
WEAK void DebugMon_Handler(void);
WEAK void PendSV_Handler(void);
WEAK void SysTick_Handler(void);
WEAK void IntDefaultHandler(void);

void DAC_IRQHandler(void) ALIAS(IntDefaultHandler);
void M0APP_IRQHandler(void) ALIAS(IntDefaultHandler);
void DMA_IRQHandler(void) ALIAS(IntDefaultHandler);
void FLASH_EEPROM_IRQHandler(void) ALIAS(IntDefaultHandler);
void ETH_IRQHandler(void) ALIAS(IntDefaultHandler);
void SDIO_IRQHandler(void) ALIAS(IntDefaultHandler);
void LCD_IRQHandler(void) ALIAS(IntDefaultHandler);
void USB0_IRQHandler(void) ALIAS(IntDefaultHandler);
void USB1_IRQHandler(void) ALIAS(IntDefaultHandler);
void SCT_IRQHandler(void) ALIAS(IntDefaultHandler);
void RIT_IRQHandler(void) ALIAS(IntDefaultHandler);
void TIMER0_IRQHandler(void) ALIAS(IntDefaultHandler);
void TIMER1_IRQHandler(void) ALIAS(IntDefaultHandler);
void TIMER2_IRQHandler(void) ALIAS(IntDefaultHandler);
void TIMER3_IRQHandler(void) ALIAS(IntDefaultHandler);
void MCPWM_IRQHandler(void) ALIAS(IntDefaultHandler);
void ADC0_IRQHandler(void) ALIAS(IntDefaultHandler);
void I2C0_IRQHandler(void) ALIAS(IntDefaultHandler);
void SPI_IRQHandler(void) ALIAS(IntDefaultHandler);
void I2C1_IRQHandler(void) ALIAS(IntDefaultHandler);
void ADC1_IRQHandler(void) ALIAS(IntDefaultHandler);
void SSP0_IRQHandler(void) ALIAS(IntDefaultHandler);
void SSP1_IRQHandler(void) ALIAS(IntDefaultHandler);
void UART0_IRQHandler(void) ALIAS(IntDefaultHandler);
void UART1_IRQHandler(void) ALIAS(IntDefaultHandler);
void UART2_IRQHandler(void) ALIAS(IntDefaultHandler);
void UART3_IRQHandler(void) ALIAS(IntDefaultHandler);
void I2S0_IRQHandler(void) ALIAS(IntDefaultHandler);
void I2S1_IRQHandler(void) ALIAS(IntDefaultHandler);
void SPIFI_IRQHandler(void) ALIAS(IntDefaultHandler);
void SGPIO_IRQHandler(void) ALIAS(IntDefaultHandler);
void GPIO0_IRQHandler(void) ALIAS(IntDefaultHandler);
void GPIO1_IRQHandler(void) ALIAS(IntDefaultHandler);
void GPIO2_IRQHandler(void) ALIAS(IntDefaultHandler);
void GPIO3_IRQHandler(void) ALIAS(IntDefaultHandler);
void GPIO4_IRQHandler(void) ALIAS(IntDefaultHandler);
void GPIO5_IRQHandler(void) ALIAS(IntDefaultHandler);
void GPIO6_IRQHandler(void) ALIAS(IntDefaultHandler);
void GPIO7_IRQHandler(void) ALIAS(IntDefaultHandler);
void GINT0_IRQHandler(void) ALIAS(IntDefaultHandler);
void GINT1_IRQHandler(void) ALIAS(IntDefaultHandler);
void EVRT_IRQHandler(void) ALIAS(IntDefaultHandler);
void CAN1_IRQHandler(void) ALIAS(IntDefaultHandler);
void ADCHS_IRQHandler(void) ALIAS(IntDefaultHandler);
void ATIMER_IRQHandler(void) ALIAS(IntDefaultHandler);
void RTC_IRQHandler(void) ALIAS(IntDefaultHandler);
void WDT_IRQHandler(void) ALIAS(IntDefaultHandler);
void M0SUB_IRQHandler(void) ALIAS(IntDefaultHandler);
void CAN0_IRQHandler(void) ALIAS(IntDefaultHandler);
void QEI_IRQHandler(void) ALIAS(IntDefaultHandler);


extern int main(void);
extern void _estack(void);


__attribute__ ((section(".vectors")))

void (* const _vectors[])(void) = {

	&_estack,                       // The initial stack pointer
	_boot,                          // The reset handler
	NMI_Handler,                    // The NMI handler
	HardFault_Handler,              // The hard fault handler
	MemManage_Handler,              // The MPU fault handler
	BusFault_Handler,               // The bus fault handler
	UsageFault_Handler,             // The usage fault handler
	(void *)0x12345678,             // CRC
	0,                              // Reserved
	0,                              // Reserved
	0,                              // Reserved
	SVC_Handler,                    // SVCall handler
	DebugMon_Handler,               // Debug monitor handler
	0,                              // Reserved
	PendSV_Handler,                 // The PendSV handler
	SysTick_Handler,                // The SysTick handler

	DAC_IRQHandler,                 // 16
	M0APP_IRQHandler,               // 17
	DMA_IRQHandler,                 // 18
	0,                              // 19
	FLASH_EEPROM_IRQHandler,        // 20
	ETH_IRQHandler,                 // 21
	SDIO_IRQHandler,                // 22
	LCD_IRQHandler,                 // 23
	USB0_IRQHandler,                // 24
	USB1_IRQHandler,                // 25
	SCT_IRQHandler,                 // 26
	RIT_IRQHandler,                 // 27
	TIMER0_IRQHandler,              // 28
	TIMER1_IRQHandler,              // 29
	TIMER2_IRQHandler,              // 30
	TIMER3_IRQHandler,              // 31
	MCPWM_IRQHandler,               // 32
	ADC0_IRQHandler,                // 33
	I2C0_IRQHandler,                // 34
	I2C1_IRQHandler,                // 35
	SPI_IRQHandler,                 // 36
	ADC1_IRQHandler,                // 37
	SSP0_IRQHandler,                // 38
	SSP1_IRQHandler,                // 39
	UART0_IRQHandler,               // 40
	UART1_IRQHandler,               // 41
	UART2_IRQHandler,               // 42
	UART3_IRQHandler,               // 43
	I2S0_IRQHandler,                // 44
	I2S1_IRQHandler,                // 45
	SPIFI_IRQHandler,               // 46
	SGPIO_IRQHandler,               // 47
	GPIO0_IRQHandler,               // 48
	GPIO1_IRQHandler,               // 49
	GPIO2_IRQHandler,               // 50
	GPIO3_IRQHandler,               // 51
	GPIO4_IRQHandler,               // 52
	GPIO5_IRQHandler,               // 53
	GPIO6_IRQHandler,               // 54
	GPIO7_IRQHandler,               // 55
	GINT0_IRQHandler,               // 56
	GINT1_IRQHandler,               // 57
	EVRT_IRQHandler,                // 58
	CAN1_IRQHandler,                // 59
	0,                              // 60
	ADCHS_IRQHandler,               // 61
	ATIMER_IRQHandler,              // 62
	RTC_IRQHandler,                 // 63
	0,                              // 64
	WDT_IRQHandler,                 // 65
	M0SUB_IRQHandler,               // 66
	CAN0_IRQHandler,                // 67
	QEI_IRQHandler,                 // 68
};


void _boot(void) 
{
	/* Let there be light */

	LPC_GPIO_PORT->DIR[1] = 1UL << 11;
	LPC_GPIO_PORT->B[1][11] = 0;

	/* Init vector table */

	uint32_t * pSCB_VTOR = (uint32_t *)0xE000ED08;
	*pSCB_VTOR = (uint32_t)&_vectors;

	/* Clear the BSS */

	extern uint8_t _sbss;
	extern uint8_t _ebss;
	register uint8_t *dst = &_sbss;
	while(dst < &_ebss) *dst++ = 0;

	/* Start and never return */
	
	main();
	for(;;);
}



void NMI_Handler(void) 
{
	for(;;);
}


void HardFault_Handler(void) 
{
	for(;;);
}


void MemManage_Handler(void) 
{
	for(;;);
}


void BusFault_Handler(void) 
{
	for(;;);
}


void UsageFault_Handler(void) 
{
	for(;;);
}


void SVC_Handler(void) 
{
	for(;;);
}


void DebugMon_Handler(void) 
{
	for(;;);
}


void PendSV_Handler(void) 
{
	for(;;);
}


void SysTick_Handler(void) 
{
	for(;;);
}


void IntDefaultHandler(void) 
{
	for(;;);
}


/*
 * End
 */
