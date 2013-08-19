/*
 * uart.h
 *
 *  Created on: Jul 27, 2013
 *      Author: cwoodall
 */

#ifndef UART_H_
#define UART_H_
#include <msp430.h>
#include <stdint.h>

#define SET_BAUDRATE(baud) (16000000L/baud) // N = f_clk/Baudrate
#define UART_RXIE 0x01
#define UART_TXIE 0x02
void setupUART(uint16_t baudrate, uint16_t settings );

#endif /* UART_H_ */
