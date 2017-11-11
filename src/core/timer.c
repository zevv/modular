
#include "chip.h"
#include "timer.h"
#include "evq.h"


static volatile uint32_t ticks = 0;

void timer_init(void)
{
        uint32_t timerFreq = Chip_Clock_GetRate(CLK_MX_TIMER0);

        Chip_TIMER_Reset(LPC_TIMER0);
        Chip_TIMER_MatchEnableInt(LPC_TIMER0, 1);
        Chip_TIMER_SetMatch(LPC_TIMER0, 1, (timerFreq / HZ));
        Chip_TIMER_ResetOnMatchEnable(LPC_TIMER0, 1);
        Chip_TIMER_Enable(LPC_TIMER0);

        NVIC_EnableIRQ(TIMER0_IRQn);
        NVIC_ClearPendingIRQ(TIMER0_IRQn);
}


uint32_t timer_get_ticks(void)
{
	return ticks;
}


void TIMER0_IRQHandler(void)
{
	Chip_TIMER_ClearMatch(LPC_TIMER0, 1);

	ticks ++;

	static int m = 0;
	if(++m == 10) {
		m = 0;

		event_t ev;
		ev.type = EV_TICK_10HZ;
		evq_push(&ev);

		static int n = 0;
		if(++n == 10) {
			n = 0;
			event_t ev;
			ev.type = EV_TICK_1HZ;
			evq_push(&ev);
		}
	}
}


/*
 * End
 */


