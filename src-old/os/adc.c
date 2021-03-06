
/*
 * Configure ADC0 and ADC1 burst mode. ADC0 handles channel 0..3, ADC1 handles
 * channel 0..4. All channels are averaged 4 times to perform oversampling.
 */
 
#include "board.h"
#include "printd.h"
#include "adc.h"

 
#define OVERSAMPLE 4
#define SAMPLERATE 24000

/* Scale back oversampled samples to -1.0..+1.0 range */

#define ADC_SCALE (1.0 / (2 << 14))

static volatile int32_t accum[8];
static volatile uint32_t count0, count1;


/* 
 * IRQs are triggered when the first of the 4 channels of each ADC is
 * read and are used to accumulate the ADC values for oversampling
 */

void ADC0_IRQHandler(void)
{
	accum[0] += LPC_ADC0->DR[0] & 0x0000ffff;
	accum[1] += LPC_ADC0->DR[1] & 0x0000ffff;
	accum[2] += LPC_ADC0->DR[2] & 0x0000ffff;
	accum[3] += LPC_ADC0->DR[3] & 0x0000ffff;
	count0 ++;
}


void ADC1_IRQHandler(void)
{
	accum[4] += LPC_ADC1->DR[4] & 0x0000ffff;
	accum[5] += LPC_ADC1->DR[5] & 0x0000ffff;
	accum[6] += LPC_ADC1->DR[6] & 0x0000ffff;
	accum[7] += LPC_ADC1->DR[7] & 0x0000ffff;
	count1 ++;
}


/*
 * Average the last read samples for each channel and normalize
 * to -1.0..+1.0 float
 */

uint32_t adc_read(volatile float *val)
{
	static int ret = 0;

	if(count0 && count1) {
		
		int32_t d0 = 0x8000 * count0;
		int32_t d1 = 0x8000 * count1;
		float m0 = ADC_SCALE / count0;
		float m1 = ADC_SCALE / count1;

		val[0] = (accum[0] - d0) * m0;
		val[1] = (accum[1] - d0) * m0;
		val[2] = (accum[2] - d0) * m0;
		val[3] = (accum[3] - d0) * m0;
		val[4] = (accum[4] - d1) * m1;
		val[5] = (accum[5] - d1) * m1;
		val[6] = (accum[6] - d1) * m1;
		val[7] = (accum[7] - d1) * m1;

		ret = count0;
		count0 = count1 = 0;
		accum[0] = accum[1] = accum[2] = accum[3] = 0;
		accum[4] = accum[5] = accum[6] = accum[7] = 0;
	
	}

	return ret;
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

	printd("rate = %d\n", cs.adcRate);
}


/*
 * End
 */
