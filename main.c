//***************************************************************************************
//  MSP430G2553 USCI_A UART Mode. Rx and Tx
//
//  C. Woodall < cwoodall at bu.edu >
//  Boston University, NASA MUX
//  July 2013
//  Built with Code Composer Studio v5
//***************************************************************************************
#define __DEBUG__
#include <msp430.h>
#include <stdint.h>

#include "uart/uart.h"
#include "spi/spi.h"
#include "spi/dac7512.h"
#include "spi/ad5504.h"
// -- FIXME: MOVE to include/uart/uart.h
static uint16_t settings_reg = 0x0001;

#define ERR_BADINPUT 1
int8_t hex2nibble(char hex) {
	if ((hex >= '0') && (hex <= '9')) {
		return (int) hex - '0';
	} else if ((hex >= 'a') && (hex <= 'f')) {
		return (hex - 'a') + 10;
	} else if ((hex >= 'A') && (hex <= 'F')) {
		return (hex - 'A') + 10;
	} else {
		return -ERR_BADINPUT;
	}
}

char nibble2hex(uint8_t a) {
	if ((a < 0xa)) {
		return (char) (a + '0');
	} else if ((a >= 0xa) && (a < 0x10)) {
		return (char) (a+'a')-0xa;
	}

	return '\x00';
}

const uint8_t __crc_table[256];
void crc_init(uint16_t *crc) {
	*crc = 0xffff;
}

void crc_add_byte(uint16_t *crc, uint8_t byte) {
	*crc ^= __crc_table[byte];
}

typedef struct {
	uint8_t escape;
	uint8_t frame;
} uart_magic_t;

typedef struct uart_commands {
	uint8_t wr_reg;
	uint8_t err;
	uint8_t ack;
} uart_commands_t;

const uart_magic_t uart_magic = {
	0x80,
	0x81
};

const uart_commands_t uart_commands = {
	0x84,
	0x82,
	0x83
};

typedef enum {
	IDLE,
	MESSAGE,
	ESCAPE
} uart_rx_state_t;

#define UART_BUF_LEN 64
typedef struct {
	uart_rx_state_t state;
//	uint8_t is_escaped;
	uint8_t buf[UART_BUF_LEN];
	uint8_t buf_end;
	uint16_t crc;
} uart_dev_t;

void uart_dev_init(uart_dev_t *uart) {
	uint8_t i;
	uart->state = IDLE;
	for (i = 0; i < UART_BUF_LEN; i++) {
		uart->buf[i] = 0;
	}
	uart->buf_end = 0;
	crc_init( &(uart->crc) );
}
// -- END FIXME

// Initialize the state and message
static uart_dev_t uart_dev;

static uint16_t ad5504_value_reg[16] = {
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0
};

static uint16_t dac7512_value_reg[4][4] = {
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};


static uint16_t dac7512_value = 0;
static spi_device_t ad5504;
static spi_device_t dac7512;
void main(void) {
  uint16_t ad5504_address_decoded = 0;
  uint8_t i;
  uint8_t j;

  WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer

  BCSCTL1 = CALBC1_16MHZ;		// Run at 16 MHz
  DCOCTL = CALDCO_16MHZ;		// Run at 16 MHz

  P1DIR |= BIT0;
/**  FIXME
  SPI_init();
  AD5504_init(&ad5504);
  DAC7512_init(&dac7512);
*/
  uart_dev_init(&uart_dev);
  setupUART(9600, UART_RXIE); // N = f_clk/Baudrate, Baudrate == 9600
  __bis_SR_register(GIE); // interrupts enabled

  // Enable all AD5504 channels on all AD5504s
/*
  for (i = 0; i < 4; i++) {
	ad5504_address_decoded = ad5504_addresses[i];
	AD5504_send(&ad5504, AD5504_CTRL, AD5504_CHA_ON | AD5504_CHB_ON | AD5504_CHC_ON | AD5504_CHD_ON, AD5504_WRITE, 1<<i);
   }
*/
  while(1) {
	  if (settings_reg & BIT0) {
		  P1OUT |= BIT0;
	  } else {
		  P1OUT &= ~BIT0;
	  }
/*
	for (i = 0; i < 16; i++) {
	  AD5504_send(&ad5504, (ad5504_addresses[i] & 0xff), ad5504_value_reg[i], AD5504_WRITE, ad5504_addresses[i]>>8);
	}
*/
  }
}


/**
 * Messages are made up of the following. Taking up 8 bytes each. The carriage return character \r helps with synchronization.
 * | CMD | ADDRESS_CHAR_H | ADDRESS_CHAR_L | MSG[15:12] | MSG[11:8] | MSG[7:4] | MSG[3:0] | \r |
 *
 * Everything is sent in HEX representation. For example if I wanted to write the message 0xfe3 to address 0x1 I would send the following:
 * > w01fe3\r
 *
 * CMD | DESCRIPTION           | RESPONSE
 * ----+-----------------------+---------
 *  w  | Write to fine DAC     | "!\r"
 *  r  | Read from fine DAC    | Data in register hex MSB.
 *  W  | Write to general registers | "!\r"
 *  R  | Read from general regs. | Data in register hex MSB.
 */
void simple_uart_putchar(char a) {
	while (!(IFG2 & UCA0TXIFG));
	UCA0TXBUF = a;
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{

	uint8_t rx_read;
	// Handle a UART Rx Interrupt
	if (IFG2 & UCA0RXIFG) {
		// Read in from UART peripheral and echo (for debug... will be removed)
		rx_read = UCA0RXBUF;
//		#ifdef __DEBUG__
		simple_uart_putchar(rx_read);
//		#endif

		switch ( uart_dev.state ) {
		case IDLE:
			if (rx_read == uart_magic.frame) {
				uart_dev.state = MESSAGE;
				uart_dev.buf_end = 0;
				crc_init(&(uart_dev.crc));
			}
			break;
		case MESSAGE:
			if (rx_read == uart_magic.escape) {
				uart_dev.state = ESCAPE;
				crc_add_byte( &(uart_dev.crc), rx_read);
			} else if (rx_read == uart_magic.frame) {
				// FIXME: Currently ignoring CRC
//				if ( crc_valid(uart_dev.crc) ) {
					uart_dev.buf_end -= 2; // remove CRC from message buffer
					// FIXME: Proper approach is to push onto a message stack and then parse messages outside of UART interrupt.
					//ring_buf_push(*uart_msg_buf, *uart_dev.buf, uart_dev.buf_end);
					// Parse Message

					if (uart_dev.buf[0] == uart_commands.wr_reg) {
						if (uart_dev.buf_end == 4) {
							if (uart_dev.buf[1] == 0x00) {
								settings_reg = (uint16_t)(((uint16_t)uart_dev.buf[2] << 8) | uart_dev.buf[3]);
								simple_uart_putchar(0x81);
								simple_uart_putchar(0x71);
								simple_uart_putchar(uart_dev.buf[1]);
								simple_uart_putchar(uart_dev.buf[2]);
								simple_uart_putchar(uart_dev.buf[3]);
								simple_uart_putchar(0x81);
							}
						}
					} else {
						simple_uart_putchar(0x70);
					}

//				} else {

//				}
				uart_dev.state = IDLE;
			} else {
				uart_dev.buf[uart_dev.buf_end] = rx_read;
				uart_dev.buf_end += 1;
				crc_add_byte( &(uart_dev.crc), rx_read);
			}
			break;
		case ESCAPE:
			uart_dev.buf[uart_dev.buf_end] = rx_read;
			uart_dev.buf_end += 1;
			crc_add_byte( &(uart_dev.crc), rx_read);
			uart_dev.state = MESSAGE;
		}
	}
/**	static uart_state_t rx_state = COMMAND; // Initialize rx state machine
	static uint16_t msg_i = 0;
	static uint16_t addr = 0;
	static uint16_t msg = 0;
	static char cmd = 0;
	int scratch = 0;
	// Handle a UART interrupt
	if (IFG2 & UCA0RXIFG) {
		uint8_t rx_read = UCA0RXBUF;
		while (!(IFG2 & UCA0TXIFG));
		UCA0TXBUF = rx_read;

		switch (rx_state) {
		case COMMAND:
			cmd=rx_read;
			rx_state = ADDRESS_H;
			break;
		case ADDRESS_H:
			addr = hex2nibble(rx_read) << 4;
			rx_state = ADDRESS_L;
			break;
		case ADDRESS_L:
			addr |= hex2nibble(rx_read);
			rx_state = MESSAGE_HH;
			break;
		case MESSAGE_HH:
			scratch = hex2nibble(rx_read);
			if (scratch >= 0) {
				if (msg_i == 3) {
					msg = (msg << 4) | ((uint16_t) scratch & 0xf);
					rx_state = END;
				} else {
					msg = (msg << 4) | ((uint16_t) scratch & 0xf);
					msg_i += 1;
				}
			} else {
				rx_state = END;
			}
			break;
		case END:
			if (rx_read == '\r') {
				if (cmd == 'W') {
					if (addr < 16) {
						ad5504_value_reg[addr] = msg;
					}
					while (!(IFG2 & UCA0TXIFG));
					UCA0TXBUF = nibble2hex(addr);

					while (!(IFG2 & UCA0TXIFG));
					UCA0TXBUF = '!';
					while (!(IFG2 & UCA0TXIFG));
					UCA0TXBUF = '\r';
					while (!(IFG2 & UCA0TXIFG));
					UCA0TXBUF = '\n';
				} else if (cmd == 'w') {
					if (((addr>>4) < 4) && ((addr&0xf) < 4)) {
						DAC7512_send(&dac7512, msg, addr>>4, addr&0xf);
					}
					while (!(IFG2 & UCA0TXIFG));
					UCA0TXBUF = nibble2hex(addr>>4);
					while (!(IFG2 & UCA0TXIFG));
					UCA0TXBUF = nibble2hex(addr&0xf);
					while (!(IFG2 & UCA0TXIFG));
					UCA0TXBUF = '!';
					while (!(IFG2 & UCA0TXIFG));
					UCA0TXBUF = '\r';
					while (!(IFG2 & UCA0TXIFG));
					UCA0TXBUF = '\n';
				} else {
					while (!(IFG2 & UCA0TXIFG));
					UCA0TXBUF = '?';
					while (!(IFG2 & UCA0TXIFG));
					UCA0TXBUF = '\r';
					while (!(IFG2 & UCA0TXIFG));
					UCA0TXBUF = '\n';
				}
				msg_i = 0;
				msg = 0;
				addr = 0;
				rx_state = COMMAND;

			}
			break;
		}
	}
	*/
}
