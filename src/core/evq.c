
#include <stddef.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>

#include "chip.h"
#include "board.h"
#include "romtab.h"
#include "evq.h"
#include "cmd.h"
#include "timer.h"

#define EVQ_SIZE 1024u 


struct evq {
	volatile size_t head;
	volatile size_t tail;
	volatile uint8_t data[EVQ_SIZE];
	uint32_t xruns;
	size_t used_max;

	uint8_t ticks_busy;
	int load;

};

static struct evq evq;


static size_t ev_size(evtype_t type)
{
	size_t s = 0;

	switch(type) {
		case EV_TICK_1HZ: 
			s = sizeof(struct ev_tick_1hz); 
			break;
		case EV_TICK_10HZ: 
			s = sizeof(struct ev_tick_10hz); 
			break;
		case EV_UART: 
			s = sizeof(struct ev_uart); 
			break;
		case EV_CAN: 
			s = sizeof(struct ev_can); 
			break;
		case EV_USBCLI: 
			s = sizeof(struct ev_usbcli); 
			break;
		case EV_BUTTON: 
			s = sizeof(struct ev_button); 
			break;
		case EV_BOOT: 
			s = sizeof(struct ev_boot); 
			break;
		case EV_START: 
			s = sizeof(struct ev_start); 
			break;
		default: 
			/* nothing */
			break;
	}

	if(s == 0u) {
		assert(s != 0u);
	}

	return s;
}


static size_t evq_used(void)
{
	size_t head = evq.head;
	size_t tail = evq.tail;
	size_t used = (head >= tail) ? (head - tail) : (EVQ_SIZE + head - tail);
	if(used > evq.used_max) {
		evq.used_max = used;
	}
	return used;
}


size_t evq_room(void)
{
	size_t used = evq_used();
	size_t room = (EVQ_SIZE - used) - 1u;
	return room;
}


static inline void pushb(uint8_t b)
{
	evq.data[evq.head] = b;
	evq.head++;
	if(evq.head == EVQ_SIZE) {
		evq.head = 0;
	}
}


static uint8_t popb(void)
{
	uint8_t b = evq.data[evq.tail];
	evq.tail++;
	if(evq.tail == EVQ_SIZE) {
		evq.tail = 0;
	}
	return b;
}


void evq_push(event_t *ev)
{
	size_t room = evq_room();
	size_t size = ev_size(ev->type);

	__disable_irq();

	if(room >= size) {
		pushb((uint8_t)ev->type);
		const uint8_t *p = ev->raw.data; 

		size_t i;
		for(i=0; i<(size - 1u); i++) {
			pushb(*p);
			p++;
		}
	} else {
		evq.xruns ++;
	}

	__enable_irq();
}


void evq_push_irq(event_t *ev)
{
	size_t room = evq_room();
	size_t size = ev_size(ev->type);

	if(room >= size) {
		pushb((uint8_t)ev->type);
		const uint8_t *p = ev->raw.data; 

		size_t i;
		for(i=0; i<(size - 1u); i++) {
			pushb(*p);
			p++;
		}
	} else {
		evq.xruns ++;
	}
}


void evq_pop(event_t *ev)
{
	__disable_irq();

	size_t head = evq.head;
	size_t tail = evq.tail;

	if(head != tail) {
		/*lint -e9030 -e9034 */
		ev->type = (evtype_t) popb();
		size_t size = ev_size(ev->type);
		size_t i;
		for(i=0; i<(size - 1u); i++) {
			ev->raw.data[i] = popb();
		}
	}

	__enable_irq();
}


static int call_matching_handler(struct ev_handler *h, void *ptr)
{
	event_t *ev = ptr;
	if((ev->type & h->mask) != 0u) {
		uint32_t t1 = timer_get_ticks();
		h->fn(ev, h->fndata);
		uint32_t t2 = timer_get_ticks();
		if(h->count) {
			*h->count += 1;
		}
		if(h->ticks) {
			*h->ticks += t2 - t1;
		}
	}
	return 1;
}


void evq_wait(event_t *ev)
{
	for(;;) {
		size_t head = evq.head;
		size_t tail = evq.tail;
		if(head == tail) {
			__WFI();
		} else {
			break;
		}
	}

	evq_pop(ev);

	uint32_t t1 = timer_get_ticks();
	ROMTAB_FOREACH(ev_handler, call_matching_handler, ev);
	uint32_t t2 = timer_get_ticks();
	evq.ticks_busy += t2 - t1;
}


uint8_t evq_get_load(void)
{
	return (uint8_t)evq.load;
}


static void on_ev_tick_1hz(event_t *ev, void *data)
{
	evq.load = evq.ticks_busy;
	evq.ticks_busy = 0;
}

EVQ_REGISTER(EV_TICK_1HZ, on_ev_tick_1hz);



static void show_stats(struct cmd_cli *cli)
{
	cmd_printd(cli, "load:%d ", evq_get_load());
	cmd_printd(cli, "xruns:%d ", (int)evq.xruns);
	cmd_printd(cli, "size:%d/%d/%d\n", (int)evq_used(), (int)evq.used_max, EVQ_SIZE);
}


static int do_print(struct ev_handler *h, void *ptr)
{
	struct cmd_cli *cli = ptr;
	cmd_printd(cli, "%p: %04x %p %5" PRIu32 " %5" PRIu32 "\n", h->fn, h->mask, h->fndata, 
			h->count ? *h->count : 0, 
			h->ticks ? *h->ticks : 0);
	return 1;
}


static int on_cmd_evq(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	if(argc >= 1u) {
		char cmd = argv[0][0];
		
		if(cmd == 's') {
			show_stats(cli);
			return 1;
		}

		if(cmd == 'l') {
			ROMTAB_FOREACH(ev_handler, do_print, cli);
			return 1;
		}

	}

	return 0;
}

CMD_REGISTER(evq, on_cmd_evq, "l[ist] | s[tats]");

/*
 * End
 */


