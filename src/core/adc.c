
/*
 * Configure ADC0 and ADC1 burst mode. ADC0 handles channel 0..3, ADC1 handles
 * channel 0..4. All channels are averaged 4 times to perform oversampling.
 */
 
#include "board.h"
#include "printd.h"
#include "adc.h"
#include "shared.h"

 
#define OVERSAMPLE 4
#define SAMPLERATE 24000

/* Scale back oversampled samples to -1.0..+1.0 range */

#define ADC_SCALE (1.0 / (2 << 14))

static volatile int32_t accum[8];


/* 
 * IRQs are triggered when the first of the 4 channels of each ADC is
 * read and are used to accumulate the ADC values for oversampling
 */

void ADC0_IRQHandler(void)
{
	shared->adc_in[5] = LPC_ADC0->DR[0] & 0x0000ffff;
	shared->adc_in[7] = LPC_ADC0->DR[1] & 0x0000ffff;
	shared->adc_in[2] = LPC_ADC0->DR[2] & 0x0000ffff;
	shared->adc_in[3] = LPC_ADC0->DR[3] & 0x0000ffff;
}


void ADC1_IRQHandler(void)
{
	shared->adc_in[4] = LPC_ADC1->DR[4] & 0x0000ffff;
	shared->adc_in[0] = LPC_ADC1->DR[5] & 0x0000ffff;
	shared->adc_in[6] = LPC_ADC1->DR[6] & 0x0000ffff;
	shared->adc_in[1] = LPC_ADC1->DR[7] & 0x0000ffff;
}



void adc_tick(void)
{
	//printd("%d\n", count0);
}


void adc_init(void)
{
	ADC_CLOCK_SETUP_T cs;
	int i;

	Chip_ADC_Init(LPC_ADC0, &cs);
	Chip_ADC_Init(LPC_ADC1, &cs);

	for(i=0; i<4; i++) {
		Chip_ADC_EnableChannel(LPC_ADC0, i+0, ENABLE);
		Chip_ADC_EnableChannel(LPC_ADC1, i+4, ENABLE);
	}
		
	Chip_ADC_Int_SetChannelCmd(LPC_ADC0, 0, ENABLE);
	Chip_ADC_Int_SetChannelCmd(LPC_ADC1, 4, ENABLE);

	Chip_ADC_SetBurstCmd(LPC_ADC0, ENABLE);
	Chip_ADC_SetBurstCmd(LPC_ADC1, ENABLE);
	NVIC_EnableIRQ(ADC0_IRQn);
	NVIC_EnableIRQ(ADC1_IRQn);
}


/*
 * End
 */
