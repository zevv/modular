#ifndef cdc_uart_h
#define cdc_uart_h

#include "board.h"
#include "app_usbd_cfg.h"

int cdc_uart_init(void);
void cdc_uart_tx(uint8_t c);
int cdc_uart_rx(uint8_t *c);


#endif 
