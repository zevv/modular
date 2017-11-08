#ifndef midi_h
#define midi_h

#include "board.h"
#include "app_usbd_cfg.h"

int midi_init(uint8_t id);
void midi_tick(void);
void midi_tx(uint8_t c);
int midi_rx(uint8_t *c);


#endif 
