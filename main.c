#include <math.h>

#include "board.h"
#include "uart.h"
#include "printd.h"
#include "biquad.h"
#include "osc.h"
#include "adc.h"

#define SRATE 48000
#define COUNT 0

static struct biquad lp_l[COUNT], lp_r[COUNT];
static struct osc osc;
static struct osc osc2;
static volatile uint32_t jiffies;

union sample {
	uint32_t u32;
	int16_t s16[2];
};


struct audio {
	float in[4];
	float adc[8];
	float out[2];
};

static volatile float max[4];
static volatile struct audio au;


static void audio_do(void)
{
	if(0) {
		int i;
		float sl = au.in[0];
		float sr = au.in[1];

		for(i=0; i<COUNT; i++) {
			biquad_run(&lp_l[i], sl, &sl);
		}
		for(i=0; i<COUNT; i++) {
			biquad_run(&lp_r[i], sr, &sr);
		}

		//float freq = osc_gen_linear(&osc2) * 1000 + 1500;
		//osc_set_freq(&osc, freq);
		//sl += osc_gen_nearest(&osc) * 0.001;
	}

	au.out[0] = au.adc[0] * 1.0;
	au.out[1] = au.adc[0] * 1.0;
}


volatile int count;

/*
 * Main I2S interrupt handler, running at 48Khz
 */

void I2S0_IRQHandler(void)
{
	if(Chip_I2S_GetRxLevel(LPC_I2S0) > 0) {

		/* Read and convert I2S and ADC inputs */

		union sample s;
		s.u32 = Chip_I2S_Receive(LPC_I2S0);
		
		au.in[0] = s.s16[0] / 32768.0;
		au.in[1] = s.s16[1] / 32768.0;
		count = adc_read(au.adc);

		int i;
		for(i=0; i<2; i++) {
			if(au.in[i] > max[i]) {
				max[i] = au.in[i];
			}
			if(au.adc[i] > max[i+2]) {
				max[i+2] = au.adc[i];
			}
		}

		audio_do();

		s.s16[0] = au.out[0] * 32768.0;
		s.s16[1] = au.out[1] * 32768.0;

		Chip_I2S_Send(LPC_I2S0, s.u32);
	} else {
		Chip_I2S_Send(LPC_I2S0, 0);
	}
}


float pot_to_freq(float v)
{
	return (expf(v) - 0.36787) * 7658 + 5;
}


float lin_to_exp(float v)
{
	return (expf(v) - 1) * 0.5819767068693265;
}



void SysTick_Handler(void)
{
	static float f = 0.05;
	static int dt = 1;

	jiffies ++;

	static int n = 0;

	if(++n == 5) {
		n = 0;
	
		if(f < 0.0005 || f > 0.100) {
			dt = - dt;
		}
		if(dt == 1) {
			f *= 0.99;
		} else {
			f /= 0.99;
		}
		
		if(f < 0) f = 0;
		if(f > 0.5) f = 0.5;

		f = pot_to_freq(au.in[2]);

		int i;
		for(i=0; i<COUNT; i++) {
			biquad_config(&lp_l[i], BIQUAD_TYPE_HP, f, 0.707);
			biquad_config(&lp_r[i], BIQUAD_TYPE_LP, f, 0.707);
		}
	}
}


static void i2s_init(void)
{
	I2S_AUDIO_FORMAT_T conf;
	conf.SampleRate = SRATE;
	conf.ChannelNumber = 2;
	conf.WordWidth = 16;

	Chip_I2S_Init(LPC_I2S0);
	Chip_I2S_RxConfig(LPC_I2S0, &conf);
	Chip_I2S_TxConfig(LPC_I2S0, &conf);
	Chip_I2S_TxStop(LPC_I2S0);
	Chip_I2S_DisableMute(LPC_I2S0);
	Chip_I2S_TxStart(LPC_I2S0);
		
	Chip_I2S_Int_RxCmd(LPC_I2S0, ENABLE, 1);
	Chip_I2S_Int_TxCmd(LPC_I2S0, ENABLE, 1);
	NVIC_EnableIRQ(I2S0_IRQn);
}


void adc2_init(void)
{
	ADC_CLOCK_SETUP_T cs;
	Chip_ADC_Init(LPC_ADC0, &cs);
	Chip_ADC_Init(LPC_ADC1, &cs);

	Chip_ADC_EnableChannel(LPC_ADC0, 0, ENABLE);
	Chip_ADC_EnableChannel(LPC_ADC1, 1, ENABLE);
}


int main(void)
{
	SystemCoreClockUpdate();
	uart_init();
	Board_Init();
	Board_Audio_Init(LPC_I2S0, UDA1380_LINE_IN);
	SysTick_Config(SystemCoreClock / 1000);

	NVIC_SetPriority(ADC1_IRQn, 1);
	NVIC_SetPriority(ADC0_IRQn, 1);
	NVIC_SetPriority(I2S0_IRQn, 1);
	
	adc_init();
	printd("Hello\n");

	int i;
	for(i=0; i<COUNT; i++) {
		biquad_init(&lp_l[i], SRATE);
		biquad_init(&lp_r[i], SRATE);
	}

	osc_init(&osc2, 1.05, SRATE);
	osc_init(&osc, 1000, SRATE);

	if(1) i2s_init();
	
	uint32_t njiffies = jiffies + 100;
	uint32_t n = 0;

	/* This loop takes 6 cycles if jiffies != njiffies. This is used to get
	 * an estimation of the time *not* spent in this loop, e.g, time spent
	 * in the interrupts doing real work */

	for(;;) {
		if(jiffies >= njiffies) {

			if(1) {
				int load = 100 * (1.0 - 10 * (float)n / SystemCoreClock * 6.0);
				printd("%3d%% ", load);
				n = 0;

				uart_tx('|');

				for(i=0; i<4; i++) {
					int j;
					int k = sqrtf(max[i]) * 10;
					for(j=0; j<10; j++) {
						uart_tx(j <= k ? '=' : ' ');
					}
					uart_tx('|');
					max[i] = 0.0;
				}
			}
			uart_tx('\n');

			adc_tick();
			printd("%d\n", count);
			njiffies += 100;
		}
		n++;
	};

	return 0;
}

/*
 * End
 */

