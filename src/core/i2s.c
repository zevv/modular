
#include <stdlib.h>
#include "board.h"
#include "i2s.h"
#include "printd.h"
#include "shared.h"
#include "adc.h"

union sample {
	uint32_t u32;
	int16_t s16[2];
};

extern bool m4_active;


STATIC const PINMUX_GRP_T mux[] = {
	{0x3, 0,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)}, /* MCLK */
	{0x6, 0,  (SCU_PINIO_FAST | SCU_MODE_FUNC4)}, /* RX SCK */
	{0x7, 2,  (SCU_PINIO_FAST | SCU_MODE_FUNC2)}, /* TX SDA */
	{0x6, 2,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)}, /* RX_SDA */
	{0x7, 1,  (SCU_PINIO_FAST | SCU_MODE_FUNC2)}, /* TX_WS */
	{0x6, 1,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)}, /* RX_WS */
};


void i2s_init(int srate)
{
	I2S_AUDIO_FORMAT_T conf;
	conf.SampleRate = srate;
	conf.ChannelNumber = 2;
	conf.WordWidth = 32;
        
	Chip_SCU_SetPinMuxing(mux, sizeof(mux) / sizeof(mux[0]));

	Chip_I2S_Init(LPC_I2S0);
	Chip_I2S_RxConfig(LPC_I2S0, &conf);
	Chip_I2S_TxConfig(LPC_I2S0, &conf);

	/*
	 * Configure I2S clocks to generate a 18.432Mhz master clock on
	 * MCLK_OUT for the codec chip:
	 *
	 * 204E6 * 15 / 83 / 2 = 18.433734 Mhz
	 *
	 * This clock is further divided by 6 (5+1) to generate the bit clock
	 * at 3.072289Mhz. With 2 channels 32 bits each, this results in a
	 * sample rate of 3.07289/32/2, yielding a sample rate of
	 * 48004.515625Hz, which is not quite 48Khz but good enough.
	 *
	 * This is not as bad as it seems. For example, for a 440Hz tone this
	 * will result in a error of 0.04Hz
	 */

	Chip_I2S_SetTxXYDivider(LPC_I2S0, 15, 83);
	Chip_I2S_SetTxBitRate(LPC_I2S0, 5);
	Chip_I2S_SetRxXYDivider(LPC_I2S0, 15, 83);
	Chip_I2S_SetRxBitRate(LPC_I2S0, 5);

	Chip_I2S_TxModeConfig(LPC_I2S0, 0, 0, I2S_RXMODE_MCENA);

#if 0
	Chip_I2S_RxSlave(LPC_I2S0);
        Chip_I2S_SetRxXYDivider(LPC_I2S0, 0, 0);
        Chip_I2S_SetRxBitRate(LPC_I2S0, 0);
	Chip_I2S_RxModeConfig(LPC_I2S0, 0, I2S_RXMODE_4PIN_ENABLE, 0);
	Chip_I2S_TxSlave(LPC_I2S0);
        Chip_I2S_SetTxXYDivider(LPC_I2S0, 0, 0);
	Chip_I2S_TxModeConfig(LPC_I2S0, 0, I2S_TXMODE_4PIN_ENABLE, 0);
        Chip_I2S_SetTxBitRate(LPC_I2S0, 0);

#endif

	Chip_I2S_TxStop(LPC_I2S0);
	Chip_I2S_DisableMute(LPC_I2S0);
	Chip_I2S_TxStart(LPC_I2S0);
		
	Chip_I2S_Int_RxCmd(LPC_I2S0, ENABLE, 1);
	Chip_I2S_Int_TxCmd(LPC_I2S0, ENABLE, 0);
	NVIC_EnableIRQ(I2S0_IRQn);
}

/*
 * Main I2S interrupt handler. Reads data from the I2S bus and passes this to 
 * the M4 if it is awake.
 */

void I2S0_IRQHandler(void)
{
	if(Chip_I2S_GetRxLevel(LPC_I2S0) > 0) {

		shared->i2s_in = Chip_I2S_Receive(LPC_I2S0);
		adc_read(shared->adc_in);

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

