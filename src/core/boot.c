
#include "chip.h"
#include "printd.h"
#include "uart.h"

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

void RTC_IRQHandler(void) ALIAS(IntDefaultHandler);
void M4_IRQHandler(void) ALIAS(IntDefaultHandler);
void DMA_IRQHandler(void) ALIAS(IntDefaultHandler);
void FLASHEEPROM_IRQHandler(void) ALIAS(IntDefaultHandler);
void ETH_IRQHandler(void) ALIAS(IntDefaultHandler);
void SDIO_IRQHandler(void) ALIAS(IntDefaultHandler);
void LCD_IRQHandler(void) ALIAS(IntDefaultHandler);
void USB0_IRQHandler(void) ALIAS(IntDefaultHandler);
void USB1_IRQHandler(void) ALIAS(IntDefaultHandler);
void SCT_IRQHandler(void) ALIAS(IntDefaultHandler);
void RIT_IRQHandler(void) ALIAS(IntDefaultHandler);
void TIMER0_IRQHandler(void) ALIAS(IntDefaultHandler);
void GINT1_IRQHandler(void) ALIAS(IntDefaultHandler);
void GPIO4_IRQHandler(void) ALIAS(IntDefaultHandler);
void TIMER3_IRQHandler(void) ALIAS(IntDefaultHandler);
void MCPWM_IRQHandler(void) ALIAS(IntDefaultHandler);
void ADC0_IRQHandler(void) ALIAS(IntDefaultHandler);
void I2C0_IRQHandler(void) ALIAS(IntDefaultHandler);
void SGPIO_IRQHandler(void) ALIAS(IntDefaultHandler);
void SPI_IRQHandler (void) ALIAS(IntDefaultHandler);
void ADC1_IRQHandler(void) ALIAS(IntDefaultHandler);
void SSP0_IRQHandler(void) ALIAS(IntDefaultHandler);
void EVRT_IRQHandler(void) ALIAS(IntDefaultHandler);
void UART0_IRQHandler(void) ALIAS(IntDefaultHandler);
void UART1_IRQHandler(void) ALIAS(IntDefaultHandler);
void UART2_IRQHandler(void) ALIAS(IntDefaultHandler);
void UART3_IRQHandler(void) ALIAS(IntDefaultHandler);
void I2S0_IRQHandler(void) ALIAS(IntDefaultHandler);
void CAN0_IRQHandler(void) ALIAS(IntDefaultHandler);
void SPIFI_ADCHS_IRQHandler(void) ALIAS(IntDefaultHandler);
void M0SUB_IRQHandler(void) ALIAS(IntDefaultHandler);


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

	// M0 Inerrupts

	RTC_IRQHandler,                 // 16 RTC
	M4_IRQHandler,                  // 17 CortexM4/M0 (LPC43XX ONLY)
	DMA_IRQHandler,                 // 18 General Purpose DMA
	0,                              // 19 Reserved
	FLASHEEPROM_IRQHandler,         // 20 ORed flash Bank A, flash Bank B, EEPROM interrupts
	ETH_IRQHandler,                 // 21 Ethernet
	SDIO_IRQHandler,                // 22 SD/MMC
	LCD_IRQHandler,                 // 23 LCD
	USB0_IRQHandler,                // 24 USB0
	USB1_IRQHandler,                // 25 USB1
	SCT_IRQHandler,                 // 26 State Configurable Timer
	RIT_IRQHandler,                 // 27 ORed Repetitive Interrupt Timer, WWDT
	TIMER0_IRQHandler,              // 28 Timer0
	GINT1_IRQHandler,               // 29 GINT1
	GPIO4_IRQHandler,               // 30 GPIO4
	TIMER3_IRQHandler,              // 31 Timer 3
	MCPWM_IRQHandler,               // 32 Motor Control PWM
	ADC0_IRQHandler,                // 33 A/D Converter 0
	I2C0_IRQHandler,                // 34 ORed I2C0, I2C1
	SGPIO_IRQHandler,               // 35 SGPIO (LPC43XX ONLY)
	SPI_IRQHandler,                 // 36 ORed SPI, DAC (LPC43XX ONLY)
	ADC1_IRQHandler,                // 37 A/D Converter 1
	SSP0_IRQHandler,                // 38 ORed SSP0, SSP1
	EVRT_IRQHandler,                // 39 Event Router
	UART0_IRQHandler,               // 40 UART0
	UART1_IRQHandler,               // 41 UART1
	UART2_IRQHandler,               // 42 UART2
	UART3_IRQHandler,               // 43 USRT3
	I2S0_IRQHandler,                // 44 ORed I2S0, I2S1
	CAN0_IRQHandler,                // 45 C_CAN0
	SPIFI_ADCHS_IRQHandler,         // 46 SPIFI OR ADCHS interrupt
	M0SUB_IRQHandler,               // 47 M0SUB core
};


void _boot(void) 
{
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


extern uint8_t _stext;
extern uint8_t _etext;
extern uint8_t _sdata;
extern uint8_t _edata;
extern uint8_t _sbss;
extern uint8_t _ebss;

static void boom(void)
{
	uint32_t j;
	uint32_t *sp = &j;

	printd("sp: %08x\n\n", sp);

	uint32_t *p = sp;
	while(p < (uint32_t *)&_estack) {
		if(*p >= (uint32_t)&_stext && *p < (uint32_t)&_etext) {
			printd("%08x\n", *p);
		}
		p++;
	}

	printd("\n");

	for(j=0; j<32; j++) {
		printd("%08x ", *(sp+j));
		if((j % 8) == 7) printd("\n");
	}

	for(;;);
}


#define D printd_set_handler(uart_tx); printd("*** %s ***\n", __FUNCTION__); boom();

void NMI_Handler(void)        { D } 
void HardFault_Handler(void)  { D } 
void MemManage_Handler(void)  { D } 
void BusFault_Handler(void)   { D } 
void UsageFault_Handler(void) { D } 
void SVC_Handler(void)        { D } 
void DebugMon_Handler(void)   { D } 
void PendSV_Handler(void)     { D } 
void IntDefaultHandler(void)  { D } 

/* * End */
