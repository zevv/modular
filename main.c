#include <math.h>

#include "board.h"
#include "biquad.h"

struct biquad lp_l, lp_r;

void poo(void)
{
        Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 1, 12);
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, 1, 12, (bool) false);
	for(;;);
}


union sample {
	uint32_t u32;
	int16_t s16[2];
};


void I2S0_IRQHandler(void)
{
	if(Chip_I2S_GetRxLevel(LPC_I2S0) > 0) {
		union sample s;
		s.u32 = Chip_I2S_Receive(LPC_I2S0);
		
		double sl = s.s16[0];
		double sr = s.s16[1];

		biquad_run(&lp_l, sl, &sl);
		biquad_run(&lp_r, sr, &sr);

		s.s16[0] = sl;
		s.s16[1] = sr;

		Chip_I2S_Send(LPC_I2S0, s.u32);
	} else {
		Chip_I2S_Send(LPC_I2S0, 0);
	}
}


int main(void)
{
	I2S_AUDIO_FORMAT_T audio_Confg;
	audio_Confg.SampleRate = 48000;
	audio_Confg.ChannelNumber = 2;
	audio_Confg.WordWidth = 16;

	SystemCoreClockUpdate();
	Board_Init();

	biquad_init(&lp_l);
	biquad_init(&lp_r);

	Chip_I2S_Init(LPC_I2S0);
	Chip_I2S_RxConfig(LPC_I2S0, &audio_Confg);
	Chip_I2S_TxConfig(LPC_I2S0, &audio_Confg);
	Chip_I2S_TxStop(LPC_I2S0);
	Chip_I2S_DisableMute(LPC_I2S0);
	Chip_I2S_TxStart(LPC_I2S0);
		
	Chip_I2S_Int_RxCmd(LPC_I2S0, ENABLE, 1);
	Chip_I2S_Int_TxCmd(LPC_I2S0, ENABLE, 1);
	NVIC_EnableIRQ(I2S0_IRQn);

	double f = 0.05;
	int dt = 1;

	for(;;) {
		Board_LED_Toggle(1);
		volatile int i;
		if(f < 0.001 || f > 0.100) {
			dt = - dt;
		}
		if(dt == 1) {
			f *= 0.96;
		} else {
			f /= 0.96;
		}
		
		biquad_config(&lp_l, BIQUAD_TYPE_BP, f, 3);
		biquad_config(&lp_r, BIQUAD_TYPE_BP, f, 3);

		for(i=0; i<100000; i++);
	}

	return 0;
}

/*
 * End
 */

