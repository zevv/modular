#ifndef can_h
#define can_h

void can_init(uint32_t can_id);

int can_uart_rx(uint8_t *c);
void can_uart_tx(uint8_t c);

#endif

