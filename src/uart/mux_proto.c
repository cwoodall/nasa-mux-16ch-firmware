/*
 * mux_proto.c: Implementation of the UART protocol which is being used.
 *
 *  Created on: Aug 26, 2013
 *      Author: Chris J. Woodall
 */
#include "uart/uart.h"
#include "uart/mux_proto.h"
#include "types/vector/vector_uint8.h"
#include "crc16/crc16.h"
void uart_dev_init(uart_dev_t *uart) {
	uart->state = IDLE;
	vector_uint8_init(&uart->rxbuf);
	vector_uint8_init(&uart->txbuf);
	crc_init( &(uart->txcrc) );
}

void uart_dev_send_tx(uart_dev_t *uart) {
	simple_uart_putchar(UART_MAGIC_FRAME_START);
	crc_init( &uart->txcrc );
	uint8_t i = 0;
	uint8_t current_byte = 0;
	for (i = 0; i < uart->txbuf.end; i++) {
		current_byte = vector_uint8_get(&uart->txbuf, i);
		if ((current_byte == UART_MAGIC_ESCAPE) || (current_byte == UART_MAGIC_FRAME_START) || (current_byte == UART_MAGIC_FRAME_END)) {
			crc_add_byte(&uart->txcrc, UART_MAGIC_ESCAPE);
			simple_uart_putchar(UART_MAGIC_ESCAPE);

		}
		crc_add_byte(&uart->txcrc, current_byte);
		simple_uart_putchar(current_byte);
	}
	for (i = 0; i < 2; i++) {
		current_byte = uart->txcrc >> i*8;
		if ((current_byte == UART_MAGIC_ESCAPE) || (current_byte == UART_MAGIC_FRAME_START) || (current_byte == UART_MAGIC_FRAME_END)) {
			simple_uart_putchar(UART_MAGIC_ESCAPE);
		}
		simple_uart_putchar(current_byte);
	}
	simple_uart_putchar(UART_MAGIC_FRAME_END);
}

void uart_dev_send_err(uart_dev_t *uart, uint8_t code) {
	vector_uint8_clear(&uart->txbuf);
	vector_uint8_push_back(&uart->txbuf, UART_COMMANDS_ERR);
	vector_uint8_push_back(&uart->txbuf, code);
	uart_dev_send_tx(uart);
}

void uart_dev_send_ack(uart_dev_t *uart) {
	vector_uint8_clear(&uart->txbuf);
	vector_uint8_push_back(&uart->txbuf, UART_COMMANDS_ACK);
	uart_dev_send_tx(uart);
}

void uart_dev_send_read_ack(uart_dev_t *uart, uint16_t reg) {
	vector_uint8_clear(&uart->txbuf);
	vector_uint8_push_back(&uart->txbuf, UART_COMMANDS_ACK);
	vector_uint8_push_back(&uart->txbuf, (uint8_t)(reg>>8));
	vector_uint8_push_back(&uart->txbuf, (uint8_t)reg);
	uart_dev_send_tx(uart);

}
