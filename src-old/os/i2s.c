
#include "board.h"
#include "os/i2s.h"
#include "os/adc.h"

#include "audio.h"
#include "module.h"

union sample {
	uint32_t u32;
	int16_t s16[2];
};


void i2s_init(int srate)
{
	I2S_AUDIO_FORMAT_T conf;
	conf.SampleRate = srate;
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

/*
 * Main I2S interrupt handler, running at 48Khz
 */

void I2S0_IRQHandler(void)
{
	if(Chip_I2S_GetRxLevel(LPC_I2S0) > 0) {

		/* Read and convert I2S and ADC inputs */

		union sample s;
		s.u32 = Chip_I2S_Receive(LPC_I2S0);
		
		au.in[0] = s.s16[0] / 32767.0;
		au.in[1] = s.s16[1] / 32767.0;
		adc_read(au.adc);

		modules_do_audio();

		/* fixme: why does this clip at 32767? */

		if(au.out[0] < -1.0) au.out[0] = -1.0;
		if(au.out[0] > +1.0) au.out[0] = +1.0;
		if(au.out[1] < -1.0) au.out[1] = -1.0;
		if(au.out[1] > +1.0) au.out[1] = +1.0;

		s.s16[0] = au.out[0] * 30000.0;
		s.s16[1] = au.out[1] * 30000.0;

		Chip_I2S_Send(LPC_I2S0, s.u32);
	} else {
		Chip_I2S_Send(LPC_I2S0, 0);
	}
}

/*
 * End
 */

