
#include <stdlib.h>
#include "board.h"
#include "i2s.h"
#include "printd.h"
#include "adc.h"

union sample {
	uint32_t u32;
	int16_t s16[2];
};

extern bool m4_active;


STATIC const PINMUX_GRP_T mux[] = {
	{0x3, 0,  (SCU_PINIO_FAST | SCU_MODE_FUNC2)},
	{0x6, 0,  (SCU_PINIO_FAST | SCU_MODE_FUNC4)},
	{0x7, 2,  (SCU_PINIO_FAST | SCU_MODE_FUNC2)},
	{0x6, 2,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)},
	{0x7, 1,  (SCU_PINIO_FAST | SCU_MODE_FUNC2)},
	{0x6, 1,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)},
};


void i2s_init(int srate)
{
	I2S_AUDIO_FORMAT_T conf;
	conf.SampleRate = srate;
	conf.ChannelNumber = 2;
	conf.WordWidth = 16;
        
	Chip_SCU_SetPinMuxing(mux, sizeof(mux) / sizeof(mux[0]));

	Chip_I2S_Init(LPC_I2S0);
	Chip_I2S_RxConfig(LPC_I2S0, &conf);
	Chip_I2S_TxConfig(LPC_I2S0, &conf);
	Chip_I2S_TxStop(LPC_I2S0);
	Chip_I2S_DisableMute(LPC_I2S0);
	Chip_I2S_TxStart(LPC_I2S0);
		
	Chip_I2S_Int_RxCmd(LPC_I2S0, ENABLE, 1);
	Chip_I2S_Int_TxCmd(LPC_I2S0, ENABLE, 0);
	NVIC_EnableIRQ(I2S0_IRQn);
}

uint32_t *i2s_val = (void *)0x1001fc00;

/*
 * Main I2S interrupt handler. Reads data from the I2S bus and passes this to 
 * the M4 if it is awake.
 */

void I2S0_IRQHandler(void)
{
	if(Chip_I2S_GetRxLevel(LPC_I2S0) > 0) {

		*i2s_val = Chip_I2S_Receive(LPC_I2S0);

		if(m4_active) {
			__SEV();
		} else {
			Chip_I2S_Send(LPC_I2S0, 0);
		}
	} else {
		Chip_I2S_Send(LPC_I2S0, 0);
	}
}


void i2s_tick(void)
{
}

/*
 * End
 */

