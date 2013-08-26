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

#include "types/vector/vector_uint8.h"
#include "uart/uart.h"
#include "uart/mux_proto.h"
#include "spi/spi.h"
#include "spi/dac7512.h"
#include "spi/ad5504.h"
#include "crc16/crc16.h"
#include "types/vector/vector_uint8.h"

// Initialize the state and message
static uint16_t settings_reg = 0x0001;
static uint8_t ignore_crc = 0;
static uart_dev_t uart_dev;
static spi_device_t ad5504;
static spi_device_t dac7512;

static uint16_t ad5504_value_reg[16] = {
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0
};

static uint16_t dac7512_value_reg[16] = {
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0
};

void main(void)
{
  uint8_t i;

  WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer

  BCSCTL1 = CALBC1_16MHZ;		// Run at 16 MHz
  DCOCTL = CALDCO_16MHZ;		// Run at 16 MHz

  P1DIR |= BIT0;
  SPI_init();
  AD5504_init(&ad5504);
  DAC7512_init(&dac7512);

  // Enable all ADC outputs and set to 0.
  for (i = 0; i < 4; i++) {

    AD5504_send(&ad5504, AD5504_CTRL, AD5504_CHA_ON | AD5504_CHB_ON | AD5504_CHC_ON | AD5504_CHD_ON, AD5504_WRITE, 1<<i);
  }

  for (i = 0; i < 16; i++) {
    AD5504_send(&ad5504, (ad5504_addresses[i] & 0xff), ad5504_value_reg[i], AD5504_WRITE, ad5504_addresses[i]>>8);
    DAC7512_send(&dac7512, dac7512_value_reg[i], (i>>2)&0x3, i&0x3);
  }

  uart_dev_init(&uart_dev);
  setupUART(9600, UART_RXIE); // N = f_clk/Baudrate, Baudrate == 9600
  __bis_SR_register(GIE); // interrupts enabled

  // Enable all AD5504 channels on all AD5504s



  while(1) {
    if (settings_reg & BIT0) {
      P1OUT |= BIT0;
    } else {
      P1OUT &= ~BIT0;
    }

    for (i = 0; i < 16; i++) {
      AD5504_send(&ad5504, (ad5504_addresses[i] & 0xff), ad5504_value_reg[i], AD5504_WRITE, ad5504_addresses[i]>>8);
      __delay_cycles(100);
      DAC7512_send(&dac7512, dac7512_value_reg[i], (i>>2)&0x3, i&0x3);
    }
  }
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
//		simple_uart_putchar(rx_read);
//		#endif

		switch ( uart_dev.state ) {
		case IDLE:
			if (rx_read == UART_MAGIC_FRAME_START) {
				uart_dev.state = MESSAGE;
				vector_uint8_clear(&uart_dev.rxbuf);
				crc_init(&(uart_dev.rxcrc));
			}
			break;
		case MESSAGE:
			if (rx_read == UART_MAGIC_ESCAPE) {
				uart_dev.state = ESCAPE;
				crc_add_byte( &(uart_dev.rxcrc), rx_read);
			} else if (rx_read == UART_MAGIC_FRAME_END) {
				if ( crc_check(uart_dev.rxcrc) || ignore_crc ) {
					// Remove CRC
				  uart_dev.rxbuf.end -= 2;
					// Find command
					switch (vector_uint8_get(&uart_dev.rxbuf, 0)) {
					case UART_COMMANDS_WR_REG: // Write Register command
						// rxbuf should be 4 long (command + addr + data)
						if (uart_dev.rxbuf.end == 4) {
						  uint8_t addr = vector_uint8_get(&uart_dev.rxbuf, 1);
							if (addr == 0x00) {
								settings_reg = (uint16_t)(((uint16_t)vector_uint8_get(&uart_dev.rxbuf, 2) << 8) | vector_uint8_get(&uart_dev.rxbuf, 3));
								uart_dev_send_ack(&uart_dev);
							} else if ( addr & 0x10) {
							  // AD5504
                ad5504_value_reg[addr & 0x0f] = (uint16_t)(((uint16_t)vector_uint8_get(&uart_dev.rxbuf, 2) << 8) | vector_uint8_get(&uart_dev.rxbuf, 3));
                uart_dev_send_ack(&uart_dev);
							} else if (addr & 0x20) {
							  // DAC7512
                dac7512_value_reg[addr & 0x0f] = (uint16_t)(((uint16_t)vector_uint8_get(&uart_dev.rxbuf, 2) << 8) | vector_uint8_get(&uart_dev.rxbuf, 3));
                uart_dev_send_ack(&uart_dev);

							} else {
								uart_dev_send_err(&uart_dev, UART_ERRORS_BAD_ADDRESS);
							}
						} else {
							uart_dev_send_err(&uart_dev, UART_ERRORS_BAD_PACKET);
						}
						break;
					case UART_COMMANDS_READ_REG:
						// rxbuf should be 2 long (command + address)
						if (uart_dev.rxbuf.end == 2) {
              uint8_t addr = vector_uint8_get(&uart_dev.rxbuf, 1);

							if (addr == 0x00) {
								uart_dev_send_read_ack(&uart_dev, settings_reg);
              } else if ( addr & 0x10) {
                uart_dev_send_read_ack(&uart_dev, ad5504_value_reg[addr & 0x0f]);
              } else if (addr & 0x20) {
                uart_dev_send_read_ack(&uart_dev, dac7512_value_reg[addr & 0x0f]);
							} else {
								uart_dev_send_err(&uart_dev, UART_ERRORS_BAD_ADDRESS);
							}
						} else {
							uart_dev_send_err(&uart_dev, UART_ERRORS_BAD_PACKET);
						}
						break;
					case UART_COMMANDS_ACK: // ACK back to ACKs... basically a cheap ping.
						uart_dev_send_ack(&uart_dev);
						break;
					case UART_COMMANDS_ERR: // DO NOT REACT TO ERRORS
						break;
					case UART_COMMANDS_DISABLE_CRC: // IGNORE CRC
            uart_dev_send_read_ack(&uart_dev, 0xDEAD);
            ignore_crc = 1;
            break;
          case UART_COMMANDS_ENABLE_CRC: // IGNORE CRC
            uart_dev_send_read_ack(&uart_dev, 0xBEEF);
            ignore_crc = 0;
            break;
					default:
						uart_dev_send_err(&uart_dev, UART_ERRORS_GENERAL);
						break;
					}
				} else {
					uart_dev_send_err(&uart_dev, UART_ERRORS_CRC);
				}
				uart_dev.state = IDLE;
			} else if (rx_read == UART_MAGIC_FRAME_START) {
				// If we have a spurious frame start, we have a FRAME error. Send a frame error then resync.
        vector_uint8_clear(&uart_dev.rxbuf);
        crc_init(&(uart_dev.rxcrc));
			  uart_dev_send_err(&uart_dev, UART_ERRORS_FRAME);
				uart_dev.state = MESSAGE;
			} else {
				vector_uint8_push_back(&(uart_dev.rxbuf), rx_read);
				crc_add_byte( &(uart_dev.rxcrc), rx_read);
			}
			break;
		case ESCAPE:
			vector_uint8_push_back(&(uart_dev.rxbuf), rx_read);
			crc_add_byte( &(uart_dev.rxcrc), rx_read);
			uart_dev.state = MESSAGE;
			break;
		}
	}
}
