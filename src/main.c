#include <math.h>
#include <string.h>

#include "os/board.h"
#include "os/uart.h"
#include "os/cdc_uart.h"
#include "os/printd.h"
#include "os/adc.h"
#include "os/spifi.h"
#include "os/i2s.h"
#include "os/cmd.h"

#include "lib/biquad.h"
#include "lib/osc.h"
#include "lib/noise.h"

#include "audio.h"
#include "module.h"
#include "panel.h"

#define SRATE 48000
#define COUNT 1

static volatile uint32_t jiffies;

static struct cmd_cli cli = {
	.rx = cdc_uart_rx,
	.tx = cdc_uart_tx,
};

static volatile float max[4];
struct audio au;


static float midi_note_freq[] = {
	8.176, 8.662, 9.177, 9.723, 10.301, 10.913, 11.562, 12.250, 12.978,
	13.750, 14.568, 15.434, 16.352, 17.324, 18.354, 19.445, 20.602, 21.827, 23.125,
	24.500, 25.957, 27.500, 29.135, 30.868, 32.703, 34.648, 36.708, 38.891, 41.203,
	43.654, 46.249, 48.999, 51.913, 55.000, 58.270, 61.735, 65.406, 69.296, 73.416,
	77.782, 82.407, 87.307, 92.499, 97.999, 103.826, 110.000, 116.541, 123.471,
	130.813, 138.591, 146.832, 155.563, 164.814, 174.614, 184.997, 195.998,
	207.652, 220.000, 233.082, 246.942, 261.626, 277.183, 293.665,
	311.127, 329.628, 349.228, 369.994, 391.995, 415.305, 440.000, 466.164,
	493.883, 523.251, 554.365, 587.330, 622.254, 659.255, 698.456, 739.989,
	783.991, 830.609, 880.000, 932.328, 987.767, 1046.502, 1108.731, 1174.659,
	1244.508, 1318.510, 1396.913, 1479.978, 1567.982, 1661.219, 1760.000, 1864.655,
	1975.533, 2093.005, 2217.461, 2349.318, 2489.016, 2637.020, 2793.826, 2959.955,
	3135.963, 3322.438, 3520.000, 3729.310, 3951.066, 4186.009, 4434.922, 4698.636,
	4978.032, 5274.041, 5587.652, 5919.911, 6271.927, 6644.875, 7040.000, 7458.620,
	7902.133, 8372.018, 8869.844, 9397.273, 9956.063, 10548.080, 11175.300,
	11839.820, 12543.850,
};


float pot_to_freq(float v)
{
	int note = v * 32 + 64;
	if(note >= 0 && note <= 127) {
		return midi_note_freq[note];
	} else {
		return 1000;
	}
}


float lin_to_exp(float v)
{
	return (expf(v) - 1) * 0.5819767068693265;
}



void SysTick_Handler(void)
{
	modules_do_tick();
	jiffies ++;
}


#define LPC_OTP                   ((LPC_OTP_T              *) LPC_OTP_BASE)


int main(void)
{
	SystemCoreClockUpdate();
	uart_init();
	Board_Init();
	Board_Audio_Init(LPC_I2S0, UDA1380_LINE_IN);
	SysTick_Config(SystemCoreClock / 1000);
	
	cdc_uart_init();
	printd_set_handler(cdc_uart_tx);

	spifi_init();
	spifi_check_update();

	if(1) adc_init();
	if(1) i2s_init(SRATE);

	NVIC_SetPriority(ADC1_IRQn, 1);
	NVIC_SetPriority(ADC0_IRQn, 1);
	NVIC_SetPriority(I2S0_IRQn, 1);

	uint32_t id = LPC_OTP->OTP0_2;
	if(id == 0x3c3ea184) panel_2_init(SRATE);
	if(id == 0x045ea184) panel_3_init(SRATE);

	printd("Hello %08x\n", LPC_OTP->OTP0_2);

	uint32_t njiffies = jiffies + 100;
	uint32_t n = 0;

	/* This loop takes 6 cycles if jiffies != njiffies. This is used to get
	 * an estimation of the time *not* spent in this loop, e.g, time spent
	 * in the interrupts doing real work */

	for(;;) {
		if(jiffies >= njiffies) {
	
			Board_LED_Toggle(0);

			if(0) {
				int load = 100 * (1.0 - 10 * (float)n / SystemCoreClock * 6.0);
				if(1) printd("%d %3d%% ", jiffies, load);
				n = 0;

				cdc_uart_tx('|');

				int i;
				for(i=0; i<4; i++) {
					int j;
					int k = sqrtf(max[i]) * 10;
					for(j=0; j<10; j++) {
						cdc_uart_tx(j < k ? '=' : ' ');
					}
					cdc_uart_tx('|');
					max[i] = 0.0;
				}

				cdc_uart_tx('\n');
			}

			modules_do_tick();
			cmd_cli_poll(&cli);

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

