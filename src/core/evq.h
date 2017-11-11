
/*
 * Event queue handling
 *
 * When adding an event type make sure to:
 *
 * - add the EV_XXXX definition
 * - define a matching ev_xxxx struct for the event data
 * - add the ev_xxxx struct to the event_t union
 * - add the new event to event_size() in evq.c
 */


#ifndef event_h
#define event_h

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "romtab.h"

enum {
	EV_BOOT       = (1 << 0),
	EV_M4         = (1 << 1),
	EV_TICK_1HZ   = (1 << 2),
	EV_TICK_10HZ  = (1 << 3),
	EV_USBCLI     = (1 << 4),
	EV_UART       = (1 << 5),
	EV_CAN        = (1 << 6),
	EV_BUTTON     = (1 << 7),
};

typedef uint8_t evtype_t;

/*
 * Data for the various event types
 */

struct ev_boot {
	evtype_t type;
} __attribute__ (( __packed__ ));

struct ev_m4 {
	evtype_t type;
} __attribute__ (( __packed__ ));

struct ev_tick_1hz {
	evtype_t type;
} __attribute__ (( __packed__ ));

struct ev_tick_10hz {
	evtype_t type;
} __attribute__ (( __packed__ ));

struct ev_usbcli {
	evtype_t type;
	uint8_t len;
	uint8_t data[8];
} __attribute__ (( __packed__ ));

struct ev_can {
	evtype_t type;
	uint32_t id: 29;
	unsigned extended: 1;
	uint8_t len;
	uint8_t data[8];
} __attribute__ (( __packed__ ));

struct ev_button {
	evtype_t type;
	bool state;
} __attribute__ (( __packed__ ));

struct ev_uart {
	evtype_t type;
	uint8_t data;
} __attribute__ (( __packed__ ));

struct ev_raw {
	evtype_t type;
	uint8_t data[1];
} __attribute__ (( __packed__ ));

/*
 * The main event type, union of all possible types.
 */

typedef union {
	evtype_t type;
	struct ev_boot boot;
	struct ev_tick_1hz tick_1hz;
	struct ev_tick_10hz tick_10hz;
	struct ev_can can;
	struct ev_usbcli usbcli;
	struct ev_button button;
	struct ev_uart uart;
	struct ev_raw raw;
} event_t;


/*
 * Event handlers
 */

typedef void (*ev_handler)(event_t *ev, void *data);

struct ev_handler {
	evtype_t mask;
	ev_handler fn;
	void *fndata;
	uint32_t *count;
	uint32_t *ticks;
};


/*
 * An instance of this structure is created in a special ELF section. At
 * runtime, the special section is treated as an array of these.
 */

#define EVQ_REGISTER(m, f)                                   \
	static uint32_t f ## _count;                         \
	static uint32_t f ## _ticks;                         \
	ROMTAB(ev_handler) {                                  \
		.mask = m,                                   \
		.fn = f,                                     \
		.count = &f ## _count,                       \
		.ticks = &f ## _ticks,                       \
	}

/*
 * Event queue API
 */

void evq_push(event_t *ev);
void evq_push_irq(event_t *ev);
void evq_wait(event_t *ev);
void evq_pop(event_t *ev);
uint8_t evq_get_load(void);
size_t evq_room(void);

#endif

