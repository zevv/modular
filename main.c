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


static float max = 0;

void I2S0_IRQHandler(void)
{
	static float t = 0;
	static float dt = 1000.0 / SRATE * M_PI * 2;

	if(Chip_I2S_GetRxLevel(LPC_I2S0) > 0) {
		union sample s;
		s.u32 = Chip_I2S_Receive(LPC_I2S0);
		
		float sl = s.s16[0];
		float sr = s.s16[1];

		sr = sl = adc_read(0) * 32000;

		if(sl > max) max = sl;
		if(sr > max) max = sr;
		max = max * 0.9;

		int i;
		for(i=0; i<COUNT; i++) {
			biquad_run(&lp_l[i], sl, &sl);
		}
		for(i=0; i<COUNT; i++) {
			biquad_run(&lp_r[i], sr, &sr);
		}
	
		float freq = osc_gen_linear(&osc2) * 1000 + 1500;
		osc_set_freq(&osc, freq);
		sl += osc_gen_nearest(&osc) * 1;

		s.s16[0] = sl;
		s.s16[1] = sr;

		t += dt;
		while(t > M_PI * 2) t -= M_PI * 2;

		Chip_I2S_Send(LPC_I2S0, s.u32);
	} else {
		Chip_I2S_Send(LPC_I2S0, 0);
	}
}


float lin_to_exp(float v)
{
	return (expf(v) - 1) * 0.5819767068693265;
}


static void update_led(void)
{
	static uint32_t i = 1;

	Board_LED_Set(1, max < i);
	
	i <<= 1;
	if(i >= 32768) i = 1;
}


void SysTick_Handler(void)
{
	static float f = 0.05;
	static int dt = 1;

	update_led();
			
	
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

		f = (adc_read(0) + 1) * 0.1;

		int i;
		for(i=0; i<COUNT; i++) {
			biquad_config(&lp_l[i], BIQUAD_TYPE_BP, f, 1.0);
			biquad_config(&lp_r[i], BIQUAD_TYPE_BP, f, 1.0);
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


int main(void)
{
	SystemCoreClockUpdate();
	uart_init();
	Board_Init();
	Board_Audio_Init(LPC_I2S0, UDA1380_LINE_IN);
	SysTick_Config(SystemCoreClock / 1000);

	adc_init();
	printd("Hello\n");

	int i;
	for(i=0; i<COUNT; i++) {
		biquad_init(&lp_l[i]);
		biquad_init(&lp_r[i]);
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

			if(0) {
				int load = 100 * (1.0 - 10 * (float)n / SystemCoreClock * 6.0);
				printd("%d%% ", load);
				int i;
				for(i=0; i<50; i++) {
					uart_tx(i*2 < load ? '#' : '-');
				}
				uart_tx('\n');
				n = 0;
			}

			adc_tick();
			njiffies += 100;
		}
		n++;
	};

	return 0;
}

/*
 * End
 */

