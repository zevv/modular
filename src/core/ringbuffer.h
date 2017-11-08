
#ifndef ringbuffer_h
#define ringbuffer_h

#include <stdint.h>
#include <stdlib.h>


struct ringbuffer {
	volatile uint32_t head;
	volatile uint32_t tail;
	volatile uint32_t size;
	uint8_t *data;
};


void rb_init(struct ringbuffer *rb, void *data, size_t size);
int rb_push(struct ringbuffer *rb, uint8_t c);
int rb_push_blocking(struct ringbuffer *rb, uint8_t c);
int rb_pop(struct ringbuffer *rb, uint8_t *c);
size_t rb_used(struct ringbuffer *rb);
size_t rb_free(struct ringbuffer *rb);

#endif
