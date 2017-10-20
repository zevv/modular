#ifndef led_h
#define led_h

#define LED_ID_BLUE LED_ID_RED

enum led_id {
	LED_ID_GREEN,
	LED_ID_YELLOW,
	LED_ID_RED,
};

enum led_state {
	LED_STATE_OFF,
	LED_STATE_ON,
};

#define LED_COUNT 3

void led_init(void);
void led_set(enum led_id id, enum led_state onoff);

#endif
