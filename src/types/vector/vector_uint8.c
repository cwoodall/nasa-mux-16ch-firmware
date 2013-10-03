/*
 * vector_uint8.c
 *
 *  Created on: Aug 26, 2013
 *      Author: Admin
 */

#include "types/vector/vector_uint8.h"
#include <stdint.h>
void vector_uint8_init(vector_uint8_t *v) {
	vector_uint8_clear(v);
	vector_uint8_unlock(v);
}

void vector_uint8_clear(vector_uint8_t *v) {
  uint8_t i = 0;
  for (i = 0; i < UART_BUF_LEN; i++) {
    v->buf[i] = 0;
  }
  v->end = 0;
}

void vector_uint8_lock(vector_uint8_t *v) {
  v->lock = 1;
}

void vector_uint8_unlock(vector_uint8_t *v) {
  v->lock = 0;
}

uint8_t vector_uint8_islocked(vector_uint8_t *v) {
  return v->lock;
}

uint8_t vector_uint8_get(vector_uint8_t *v, uint8_t i) {
	return v->buf[i];
}

uint8_t vector_uint8_len(vector_uint8_t *v) {
	return v->end;
}

int16_t vector_uint8_pop_back(vector_uint8_t *v) {
	if (v->end != 0) {
		uint8_t temp = v->buf[(v->end - 1)];
		v->end -= 1;
		return temp;
	} else {
		return -1; // EMPTY
	}
}

int8_t vector_uint8_push_back(vector_uint8_t *v, uint8_t data) {
	if (v->end != UART_BUF_LEN) {
		v->buf[v->end] = data;
		(v->end)++;
		return 0; // Success
	} else {
		return -1; // Error
	}
}
