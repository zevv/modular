#include <math.h>

#include "board.h"
#include "uart.h"
#include "printd.h"
#include "biquad.h"

#define COUNT 5

static struct biquad lp_l[COUNT], lp_r[COUNT];

static volatile uint32_t jiffies;
union sample {
	uint32_t u32;
	int16_t s16[2];
};


void I2S0_IRQHandler(void)
{
	if(Chip_I2S_GetRxLevel(LPC_I2S0) > 0) {
		union sample s;
		s.u32 = Chip_I2S_Receive(LPC_I2S0);
		
		float sl = s.s16[0];
		float sr = s.s16[1];

		int i;
		for(i=0; i<COUNT; i++) {
			biquad_run(&lp_l[i], sl, &sl);
		}
		for(i=0; i<COUNT; i++) {
			biquad_run(&lp_r[i], sr, &sr);
		}

		s.s16[0] = sl;
		s.s16[1] = sr;

		Chip_I2S_Send(LPC_I2S0, s.u32);
	} else {
		Chip_I2S_Send(LPC_I2S0, 0);
	}
}


void SysTick_Handler(void)
{
	static float f = 0.05;
	static int dt = 1;
	
	jiffies ++;

	static int n = 0;

	if(++n == 150) {
		n = 0;
	
		if(f < 0.0005 || f > 0.100) {
			dt = - dt;
		}
		if(dt == 1) {
			f *= 0.96;
		} else {
			f /= 0.96;
		}

		int i;
		for(i=0; i<COUNT; i++) {
			biquad_config(&lp_l[i], BIQUAD_TYPE_BP, f, 1);
			biquad_config(&lp_r[i], BIQUAD_TYPE_BP, f, 1);
		}
	}
}


static void i2s_init(void)
{
	I2S_AUDIO_FORMAT_T conf;
	conf.SampleRate = 48000;
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
	Board_Init();
	uart_init();
	SysTick_Config(SystemCoreClock / 1000);

	printd("Hello\n");

	int i;
	for(i=0; i<COUNT; i++) {
		biquad_init(&lp_l[i]);
		biquad_init(&lp_r[i]);
	}

	if(1) i2s_init();
	
	uint32_t njiffies = jiffies + 100;
	uint32_t n = 0;

	/* This loop takes 6 cycles if jiffies != njiffies */

	for(;;) {
		if(jiffies >= njiffies) {
			Board_LED_Toggle(1);
			int load = 100 * (1.0 - 10 * (float)n / SystemCoreClock * 6.0);
			printd("%d%% ", load);
			int i;
			for(i=0; i<50; i++) {
				uart_tx(i*2 < load ? '#' : '-');
			}
			uart_tx('\n');
			n = 0;
			njiffies += 100;
		}
		n++;
	};

	return 0;
}

/*
 * End
 */

