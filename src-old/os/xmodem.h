#ifndef xmodem_h
#define xmodem_h

typedef void (*xmodem_rxfn)(uint8_t *buf, size_t len);

size_t xmodem_receive(xmodem_rxfn rxfn, void (*tx)(uint8_t c), int (*rx)(uint8_t *c));

#endif
