
#include <stdlib.h>

#include "chip.h"
#include "led.h"
#include "cmd.h"

struct led {
        uint8_t gpio;
        uint8_t pin;
        uint8_t invert;
};


struct led led_list[] = {
        [LED_ID_GREEN]  = { 2,  8, 0 },
        [LED_ID_YELLOW] = { 0, 14, 0 },
        [LED_ID_RED]    = { 1, 11, 0 },
};

STATIC const PINMUX_GRP_T mux[] = {
        { 6, 12, (SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN | SCU_MODE_FUNC0)},
        { 2, 10, (SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN | SCU_MODE_FUNC0)},
        { 2, 11, (SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN | SCU_MODE_FUNC0)},
};


void led_set(enum led_id id, enum led_state onoff)
{
	if(id < LED_COUNT) {
		struct led *led = &led_list[id];
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, led->gpio, led->pin, !onoff);
	}
}


void led_init(void)
{
        Chip_SCU_SetPinMuxing(mux, sizeof(mux) / sizeof(mux[0]));

	size_t i;

	for(i=0; i<LED_COUNT; i++) {
                struct led *led = &led_list[i];
                Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, led->gpio, led->pin);
		led_set(i, LED_STATE_OFF);
        }
}


static int on_cmd_led(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	if(argc >= 1u) {
		char cmd = argv[0][0];

		if((cmd == 's') && (argc >= 3u)) {
			int id = atoi(argv[1]);
			int s = atoi(argv[2]);
			led_set(id, s);
			return 1;
		}
	}
	return 0;
}

CMD_REGISTER(led, on_cmd_led, "l[ist] | s[et] <id> <0|1>");


/*
 * End
 */
