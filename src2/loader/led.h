#ifndef led_h
#define led_h

enum led_id {
	LED_ID_RED,
	LED_ID_BLUE,
};

enum led_state {
	LED_STATE_OFF,
	LED_STATE_ON,
};

#define LED_COUNT 2

void led_init(void);
void led_set(enum led_id id, enum led_state onoff);

#endif
