#include "ringbuffer.h"

void rb_init(struct ringbuffer *rb, void *data, size_t size)
{
	rb->head = 0;
	rb->tail = 0;
	rb->data = data;
	rb->size = size;
}


int rb_push(struct ringbuffer *rb, uint8_t c)
{
	uint32_t head_next = (rb->head + 1) % rb->size;
	if(head_next != rb->tail) {
		rb->data[rb->head] = c;
		rb->head = head_next;
		return 1;
	} else {
		return 0;
	}
}


int rb_pop(struct ringbuffer *rb, uint8_t *c)
{
	if(rb->head != rb->tail) {
		*c = rb->data[rb->tail];
		rb->tail = (rb->tail + 1) % rb->size; 
		return 1;
	} else {
		return 0;
	}
}


size_t rb_used(struct ringbuffer *rb)
{
	if(rb->head >= rb->tail) {
		return rb->head - rb->tail;
	} else {
		return rb->size + rb->head - rb->tail;
	}
}


size_t rb_free(struct ringbuffer *rb)
{
	return rb->size - rb_used(rb) - 1;
}


/*
 * End
 */

