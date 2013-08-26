/*
 * vector_uint8.h
 *
 *  Created on: Aug 26, 2013
 *      Author: Admin
 */

#ifndef VECTOR_UINT8_H_
#define VECTOR_UINT8_H_
#include <stdint.h>

#define UART_BUF_LEN 32
typedef struct {
  uint8_t buf[UART_BUF_LEN];
  uint8_t end;
  uint8_t lock;
} vector_uint8_t;

void vector_uint8_init(vector_uint8_t *v);
void vector_uint8_clear(vector_uint8_t *v);
void vector_uint8_lock(vector_uint8_t *v);
void vector_uint8_unlock(vector_uint8_t *v);
uint8_t vector_uint8_islocked(vector_uint8_t *v);
uint8_t vector_uint8_get(vector_uint8_t *v, uint8_t i);
uint8_t vector_uint8_size(vector_uint8_t *v);
int16_t vector_uint8_pop_back(vector_uint8_t *v);
int8_t vector_uint8_push_back(vector_uint8_t *v, uint8_t data);


#endif /* VECTOR_UINT8_H_ */
