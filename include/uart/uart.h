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
#include "crc16/crc16.h"
#define SET_BAUDRATE(baud) (16000000L/baud) // N = f_clk/Baudrate
#define UART_RXIE 0x01
#define UART_TXIE 0x02

/**
 * Set up the UART peripheral
 *
 * @param  baudrate  What baudrate are we going to use
 * @param  settings  Set up basic setting using UART_RXIE, UART_TXIE, and the like.
 * @author Chris J. Woodall <cwoodall@bu.edu>
 * @date 2013-08-26
 *
 * @example setupUART(9600, UART_RXIE | UART_TXIE) // Will enable Rx and Tx
 * 												   // interrupts and run at
 * 												   // 9600 baud.
 *
 */
void setupUART(uint16_t baudrate, uint16_t settings );

/**
 * Send a single character (UART_TXIE disabled).
 *
 * @param a Character to send.
 * @author Chris J. Woodall <cwoodall@bu.edu>
 * @date 2013-08-26
 */
void simple_uart_putchar(char a);

/**
 * Send a string of bytes over uart (UART_TXIE disabled)
 *
 * @param  a  Pointer to a string of characters of length n
 * @param  n  Length of the string of characters.
 */
void simple_uart_putstr(char *a, uint8_t n);

#endif /* UART_H_ */
