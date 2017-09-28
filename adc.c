
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

#define ADC_SCALE (1.0 / (2 << 16) / OVERSAMPLE)

static uint32_t accum[8];
static uint32_t count0, count1;
static volatile float val[8];

/* 
 * IRQs are generated when the last of the 4 channels of each ADC is
 * read. Accumulate the A/D values, and on receiving every 4th value,
 * convert to float -1.0..+1.0 in val[] array
 */

static volatile uint32_t n;

void ADC0_IRQHandler(void)
{
	accum[0] += LPC_ADC0->DR[0] & 0x0000ffff;
	accum[1] += LPC_ADC0->DR[1] & 0x0000ffff;
	accum[2] += LPC_ADC0->DR[2] & 0x0000ffff;
	accum[3] += LPC_ADC0->DR[3] & 0x0000ffff;

	if(++count0 == OVERSAMPLE) {
		n ++;
		val[0] = accum[0] * ADC_SCALE - 1.0; accum[0] = 0;
		val[1] = accum[1] * ADC_SCALE - 1.0; accum[1] = 0;
		val[2] = accum[2] * ADC_SCALE - 1.0; accum[2] = 0;
		val[3] = accum[3] * ADC_SCALE - 1.0; accum[3] = 0;
		count0 = 0;
	}
}

void adc_tick(void)
{
	printd("%d\n", n * 10);
	n = 0;
}

void ADC1_IRQHandler(void)
{
	accum[4] += LPC_ADC1->DR[4] & 0x0000ffff;
	accum[5] += LPC_ADC1->DR[5] & 0x0000ffff;
	accum[6] += LPC_ADC1->DR[6] & 0x0000ffff;
	accum[7] += LPC_ADC1->DR[7] & 0x0000ffff;

	if(++count1 == OVERSAMPLE) {
		val[4] = accum[4] * ADC_SCALE - 1.0; accum[4] = 0;
		val[5] = accum[5] * ADC_SCALE - 1.0; accum[5] = 0;
		val[6] = accum[6] * ADC_SCALE - 1.0; accum[6] = 0;
		val[7] = accum[7] * ADC_SCALE - 1.0; accum[7] = 0;
		count1 = 0;
	}
}


void adc_init(void)
{
	ADC_CLOCK_SETUP_T cs;
	int i;

	Chip_ADC_Init(LPC_ADC0, &cs);
	Chip_ADC_Init(LPC_ADC1, &cs);

	/* Set clock to allow for 4 channels per A/D, oversampled at required
	 * samplerate */

	uint32_t rate = SAMPLERATE * OVERSAMPLE * 4;
	printd("rate = %d\n", rate);
	Chip_ADC_SetSampleRate(LPC_ADC0, &cs, SAMPLERATE * OVERSAMPLE * 4);
	Chip_ADC_SetSampleRate(LPC_ADC1, &cs, SAMPLERATE * OVERSAMPLE * 4);
	
	for(i=0; i<4; i++) {
		Chip_ADC_EnableChannel(LPC_ADC0, i+0, ENABLE);
		Chip_ADC_EnableChannel(LPC_ADC1, i+4, ENABLE);
	}
		
	Chip_ADC_Int_SetChannelCmd(LPC_ADC0, 3, ENABLE);
	Chip_ADC_Int_SetChannelCmd(LPC_ADC1, 7, ENABLE);

	Chip_ADC_SetBurstCmd(LPC_ADC0, ENABLE);
	Chip_ADC_SetBurstCmd(LPC_ADC1, ENABLE);
	NVIC_EnableIRQ(ADC0_IRQn);
	NVIC_EnableIRQ(ADC1_IRQn);

	printd("rate = %d\n", cs.adcRate);
}


float adc_read(int channel)
{
	if(channel >=0 && channel < 8) {
		return val[channel];
	} else {
		return 0;
	}
}


/*
 * End
 */
