
#include <stdlib.h>

#include "chip.h"
#include "button.h"
#include "cmd.h"


STATIC const PINMUX_GRP_T mux[] = {
        { 1, 1, (SCU_MODE_INBUFF_EN | SCU_MODE_FUNC0)},
};


bool button_get(void)
{
	return !Chip_GPIO_GetPinState(LPC_GPIO_PORT, 0, 8);
}


void button_init(void)
{
        Chip_SCU_SetPinMuxing(mux, sizeof(mux) / sizeof(mux[0]));
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, 0, 8);
}


/*
 * End
 */
