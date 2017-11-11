
#include "board.h"
#include "printd.h"

static void setup(int timeout)
{
	Chip_WWDT_DeInit(LPC_WWDT);
	Chip_WWDT_Init(LPC_WWDT);
	Chip_WWDT_SetTimeOut(LPC_WWDT, timeout);
	Chip_WWDT_SetWarning(LPC_WWDT, 512);
	Chip_WWDT_SetWindow(LPC_WWDT, WDT_OSC - (WDT_OSC / 10));
	Chip_WWDT_SetOption(LPC_WWDT, WWDT_WDMOD_WDRESET);
	Chip_WWDT_ClearStatusFlag(LPC_WWDT, WWDT_WDMOD_WDTOF | WWDT_WDMOD_WDINT);
	Chip_WWDT_Start(LPC_WWDT);
}


void watchdog_init(void)
{
	setup(WDT_OSC * 2);
}


void watchdog_poll(void)
{
	Chip_WWDT_Feed(LPC_WWDT);
}


void watchdog_reboot(void)
{
	setup(0);
	for(;;) printd(".");
}


/*
 * End
 */
