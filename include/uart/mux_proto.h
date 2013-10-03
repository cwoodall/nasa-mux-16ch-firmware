/*
 * mux_proto.h: Implementation of the UART protocol which is being used.
 *
 *  Created on: Aug 26, 2013
 *      Author: Chris J. Woodall
 */

#ifndef __MUX_PROTO_H__
#define __MUX_PROTO_H__
#include "types/vector/vector_uint8.h"

/**
 * Contains information of the UART_MAGIC bytes ESCAPE and FRAME.
 */
#define UART_MAGIC_ESCAPE      0x80
#define UART_MAGIC_FRAME_START 0x81
#define UART_MAGIC_FRAME_END   0x82
/**
 * Contains information of the UART_COMMANDS.
 */
#define UART_COMMANDS_ACK      0x83
#define UART_COMMANDS_ERR      0x84
#define UART_COMMANDS_WR_REG   0x85
#define UART_COMMANDS_READ_REG 0x86
#define UART_COMMANDS_DISABLE_CRC 0xf0
#define UART_COMMANDS_ENABLE_CRC  0xf1

/**
 * Contains information of the UART_MAGIC bytes ESCAPE and FRAME.
 */
#define UART_ERRORS_GENERAL     0x00
#define UART_ERRORS_CRC         0x01
#define UART_ERRORS_BAD_PACKET  0x02
#define UART_ERRORS_BAD_ADDRESS 0x03
#define UART_ERRORS_FRAME       0x04


/**
 * states for the uart state machine.
 */
typedef enum {
	IDLE,
	MESSAGE,
	ESCAPE
} uart_rx_state_t;


/**
 * UART Device
 *
 * @author Chris J. Woodall <cwoodall@bu.edu>
 * @date 2013-08-26
 */
typedef struct {
	uart_rx_state_t state;
	vector_uint8_t rxbuf;
	uint16_t rxcrc;

	vector_uint8_t txbuf;
	uint16_t txcrc;

} uart_dev_t;

/**
 * Bring the uart_dev_t to a starting state.
 *
 * @param  uart   The UART device to be initialized
 * @author Chris J. Woodall <cwoodall@bu.edu>
 * @date 2013-08-26
 */
void uart_dev_init(uart_dev_t *uart);

void uart_dev_send_tx(uart_dev_t *uart);
void uart_dev_send_err(uart_dev_t *uart, uint8_t code);
void uart_dev_send_ack(uart_dev_t *uart);
void uart_dev_send_read_ack(uart_dev_t *uart, uint16_t reg);

#endif
