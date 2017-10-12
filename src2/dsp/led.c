
#include <stdlib.h>

#include "chip.h"
#include "led.h"

struct led {
        uint8_t gpio;
        uint8_t pin;
        uint8_t invert;
};


struct led led_list[] = {
        [LED_ID_RED]  = { 1, 12, 0 },
        [LED_ID_BLUE] = { 1, 11, 0 },
};

STATIC const PINMUX_GRP_T mux[] = {
        { 2, 11, (SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN | SCU_MODE_FUNC0)},
        { 2, 12, (SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN | SCU_MODE_FUNC0)},
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


/*
 * End
 */
