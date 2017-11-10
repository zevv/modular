#ifndef usb_h
#define usb_h

#include "board.h"
#include "app_usbd_cfg.h"

int usb_init(uint8_t id);
void usb_tick(void);
void usb_clipipe_tx(uint8_t c);
int usb_clipipe_rx(uint8_t *c);


#endif 
