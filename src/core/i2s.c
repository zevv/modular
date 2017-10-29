
#include <stdlib.h>
#include <math.h>
#include "board.h"
#include "i2s.h"
#include "printd.h"
#include "shared.h"
#include "adc.h"

union sample {
	uint32_t u32;
	int16_t s16[2];
};


STATIC const PINMUX_GRP_T mux[] = {

	/* I2S0 */

	{ 3, 0,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)}, /* TX_MCLK */
	{ 6, 0,  (SCU_PINIO_FAST | SCU_MODE_FUNC4)}, /* RX SCK */
	{ 7, 2,  (SCU_PINIO_FAST | SCU_MODE_FUNC2)}, /* TX SDA */
	{ 6, 2,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)}, /* RX_SDA */
	{ 7, 1,  (SCU_PINIO_FAST | SCU_MODE_FUNC2)}, /* TX_WS */
	{ 6, 1,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)}, /* RX_WS */
	
	/* I2S1 */

	/* MCLK does not exist for I2S1 */
	/* RX SCK is at CLK2 */
	{ 0, 1,  (SCU_PINIO_FAST | SCU_MODE_FUNC7)}, /* TX SDA */
	{ 3, 4,  (SCU_PINIO_FAST | SCU_MODE_FUNC6)}, /* RX_SDA */
	{ 0, 0,  (SCU_PINIO_FAST | SCU_MODE_FUNC7)}, /* TX_WS */
	{ 3, 5,  (SCU_PINIO_FAST | SCU_MODE_FUNC6)}, /* RX_WS */
	{ 4, 7,  (SCU_PINIO_FAST | SCU_MODE_FUNC6)}, /* TX_SCK */
};


void i2s_init_one(LPC_I2S_T *i2s, int srate)
{
	I2S_AUDIO_FORMAT_T conf;
	conf.SampleRate = srate;
	conf.ChannelNumber = 2;
	conf.WordWidth = 32;
        
	Chip_I2S_RxConfig(i2s, &conf);
	Chip_I2S_TxConfig(i2s, &conf);

	/*
	 * Configure I2S clocks to generate a master clock on MCLK_OUT for the
	 * codec chip. The fractional divider will generate jitter when xDiv is
	 * not divisable by yDiv, so we have limited choices. For now we choose
	 *
	 * 204E6 * 1 / 6 / 2 = 17 Mhz MCLK
	 *
	 * This clock is further divided by 6 (5+1) to generate the bit clock
	 * at 2.83Mhz. With 2 channels 32 bits each, this results in a sample
	 * rate of 2.83/32/2, resulting in a sample rate of 44270.833Hz
	 */

	Chip_I2S_SetTxXYDivider(i2s, 1, 6);
	Chip_I2S_SetTxBitRate(i2s, 5);
	Chip_I2S_SetRxXYDivider(i2s, 1, 6);
	Chip_I2S_SetRxBitRate(i2s, 5);

	Chip_I2S_TxModeConfig(i2s, 0, 0, I2S_RXMODE_MCENA);

#if 0
	Chip_I2S_RxSlave(i2s);
        Chip_I2S_SetRxXYDivider(i2s, 0, 0);
        Chip_I2S_SetRxBitRate(i2s, 0);
	Chip_I2S_RxModeConfig(i2s, 0, I2S_RXMODE_4PIN_ENABLE, 0);
	Chip_I2S_TxSlave(i2s);
        Chip_I2S_SetTxXYDivider(i2s, 0, 0);
	Chip_I2S_TxModeConfig(i2s, 0, I2S_TXMODE_4PIN_ENABLE, 0);
        Chip_I2S_SetTxBitRate(i2s, 0);

#endif

	Chip_I2S_TxStop(i2s);
	Chip_I2S_DisableMute(i2s);
	Chip_I2S_TxStart(i2s);
		
	Chip_I2S_Int_RxCmd(i2s, ENABLE, 2);
	//Chip_I2S_Int_TxCmd(i2s, ENABLE, 0);
}


void i2s_init(int srate)
{
	Chip_SCU_SetPinMuxing(mux, sizeof(mux) / sizeof(mux[0]));
	Chip_SCU_ClockPinMuxSet(2, 7);

	Chip_Clock_Enable(CLK_APB1_I2S);

	i2s_init_one(LPC_I2S0, srate);
	i2s_init_one(LPC_I2S1, srate);

	NVIC_EnableIRQ(I2S0_IRQn);
}


/*
 * Main I2S interrupt handler. Reads data from the I2S bus and passes this to 
 * the M4 if it is awake.
 */

void I2S0_IRQHandler(void)
{
	if(Chip_I2S_GetRxLevel(LPC_I2S1) > 0) {
		shared->i2s_in[2] = Chip_I2S_Receive(LPC_I2S1);
		shared->i2s_in[3] = Chip_I2S_Receive(LPC_I2S1);
	}

	if(Chip_I2S_GetRxLevel(LPC_I2S0) > 0) {

		shared->i2s_in[0] = Chip_I2S_Receive(LPC_I2S0);
		shared->i2s_in[1] = Chip_I2S_Receive(LPC_I2S0);

		if(shared->m4_state != M4_STATE_HALT) {
			__SEV();
		} else {
			if(Chip_I2S_GetTxLevel(LPC_I2S0) < 4) {
				Chip_I2S_Send(LPC_I2S0, 0xf0ccaa);
				Chip_I2S_Send(LPC_I2S0, 0xf0ccaa);
			}
			if(Chip_I2S_GetTxLevel(LPC_I2S1) < 4) {
				Chip_I2S_Send(LPC_I2S1, 0xf0ccaa);
				Chip_I2S_Send(LPC_I2S1, 0xf0ccaa);
			}
		}
	}
}


void i2s_tick(void)
{
	static int n = 0;
	if(n ++ == 100) {
		if(0) printd("%d\n", shared->i2s_in[0], shared->i2s_in[1]);
		n = 0;
	}
}

/*
 * End
 */

