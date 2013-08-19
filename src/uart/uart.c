/*
 * uart.c
 *
 *  Created on: Jul 27, 2013
 *      Author: cwoodall
 */

#include <msp430.h>
#include <stdint.h>
#include "uart/uart.h"

void setupUART(uint16_t baudrate, uint16_t settings)
{
  // Select pin modes to Rx and Tx
  P1SEL |= BIT1 | BIT2;  // Basic setup for UART Mode
  P1SEL2 |= BIT1 | BIT2; // Tell that we are in UART MODE

  UCA0CTL1 = UCSWRST;
  UCA0CTL1 |= UCSSEL_2; // USCI_A use SMCLK

  // Setup the baudrate
  UCA0BR0 = SET_BAUDRATE(baudrate) & 0xff;
  UCA0BR1 = SET_BAUDRATE(baudrate)>>8;
  UCA0MCTL = UCBRS_1;

  // exit reset mode
  UCA0CTL1 &= ~UCSWRST;

  if (settings & UART_RXIE) {
	  IE2 |= UCA0RXIE;
  }

  if (settings & UART_TXIE) {
	  IE2 |= UCA0TXIE;
  }
}

