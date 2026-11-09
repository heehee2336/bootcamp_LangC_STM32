/*
 * que_buffer.c
 *
 *  Created on: 2026. 10. 30.
 *      Author: kccistc
 */

#include "que_buffer.h"
#include <string.h>



void que_buf_init(que_buf_t *que, void *storage, uint16_t elem_size, uint16_t capacity) {
	que -> head = 0;
	que -> tail = 0;
	que -> buf = (uint8_t *)storage;
	que -> elem_size = elem_size;
	que -> capacity = capacity;
}





int que_ary[4];


uint8_t *slot(que_buf_t *que, uint16_t n) {
	return que -> buf + (uint32_t)n * que -> elem_size;
}

uint16_t next(que_buf_t* que, uint16_t h) {
	return (uint16_t)((h + 1) % que -> capacity);
}


bool que_is_empty(que_buf_t *que) {
	return (que -> head == que -> tail);
}

bool que_is_full(que_buf_t *que) {
	return (next(que, que -> head) == que -> tail);
}


bool que_put(que_buf_t *que, void* elem) {
	uint16_t head = que -> head;
	uint16_t next_head = next(que, head);
	if (next_head == que -> tail) return false;

	memcpy(slot(que, que -> head), elem, que -> elem_size);
	que -> head = next_head;
	return true;
}

bool que_get(que_buf_t *que, void* elem) {

	if (que -> tail == que -> head) return false;

	memcpy(elem, slot(que, que -> tail), que -> elem_size);
	que -> tail = next(que, que -> tail);
	return true;
}
