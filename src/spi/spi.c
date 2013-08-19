/*
 * spi.c
 *
 *  Created on: Jun 21, 2013
 *      Author: Christopher Woodall
 */
#include "spi/spi.h"
#include <msp430.h>
#include <stdint.h>

void SPI_init()
{
#ifdef __MSP430_HAS_USI__
	P1DIR   |=  PIN_MOSI + PIN_SCLK;
	P1DIR 	&= ~PIN_MISO;

	//// setup SPI
	USICTL0   |= USISWRST;                     // put USI in reset mode, source USI clock from SMCLK
	USICTL0  |= USIPE5 | USIPE6 | USIPE7 | USIMST | USIOE | USIGE;
	USICKCTL  |= USIDIV_1 | USISSEL_2;         // default speed 16MHz/1
	USICTL1    |= USICKPH;                      // SPI_MODE_0, CPHA=0
	USICKCTL  &= ~USICKPL;       // CPOL=0
	USICTL0  &= ~USILSB;       // MSBFIRST
	USICTL0  &= ~USISWRST;           // release for operation
#endif

#ifdef __MSP430_HAS_USCI__
	// Disable USCI Peripheral and clocked from SMCLK
	UCB0CTL1 = UCSWRST | UCSSEL_2;
	// 8 bit (default), 3-wire (default, mode 0), synchronous, MSB first. MASTER MODE
	UCB0CTL0 = UCMSB | UCMST | UCSYNC | UCMODE_0;

	// Setup output pins
	P1OUT  |= (BIT5 | BIT7);            // SPI output pins low
	P1SEL  |= BIT5 | BIT6 | BIT7;	    // configure P1.5, P1.6, P1.7 for USCI
	P1SEL2 |= BIT5 | BIT6 | BIT7;       // more required SPI configuration

	// Default to SMCLK/16
	UCB0BR0 = SPI_CLOCK_DIV16 & 0xff;
	UCB0BR1 = (SPI_CLOCK_DIV16 >> 8) & 0xff;

	UCB0CTL1 &= ~UCSWRST; // Enable SPI peripheral
#endif
}

/**
 * Set the SPI mode, spi.h defines SPI_MODE_[0..3] and CPHA_bit and CPOL_but for use in configuration
 */
inline void SPI_set_mode(uint8_t spi_mode) {
#ifdef __MSP430_HAS_USCI__
	UCB0CTL1 |= UCSWRST;        // go into reset state

	UCB0CTL0 = (UCB0CTL0 & ~SPI_MODE_MASK) | spi_mode;

	UCB0CTL1 &= ~UCSWRST;        // go into reset state
#endif

#ifdef __MSP430_HAS_USI__
	if (spi_mode & CPHA_bit) {
		USICTL1    &= ~USICKPH;                      // CPHA=1
	} else {
		USICTL1    |= USICKPH;                      // CPHA=0
	}

	if (spi_mode & CPOL_bit) {
		USICKCTL  |= USICKPL;       // CPOL=1
	} else {
		USICKCTL  &= ~USICKPL;       // CPOL=0
	}
#endif
}

/**
 * Generic function for sending SPI data, relatively slow throughput with this
 * particular method because SPI mode gets set every time a spi send occurs.
 */
uint16_t SPI_send(spi_device_t *spi_dev, uint32_t value, uint16_t cs_arg) {
#ifdef __MSP430_HAS_USCI__
	SPI_set_mode(spi_dev->spi_mode);
	spi_dev->chip_select(cs_arg);

	if (spi_dev->bits == 16) {
		UCB0TXBUF = (value >> 8) & 0xff; // setting TXBUF clears the TXIFG flag
		while (UCB0STAT & UCBUSY); // wait for SPI TX/RX to finish
		UCB0TXBUF = (value) & 0xff; // setting TXBUF clears the TXIFG flag
		while (UCB0STAT & UCBUSY); // wait for SPI TX/RX to finish
	} else if (spi_dev->bits == 8) {
		UCB0TXBUF = (value) & 0xff; // setting TXBUF clears the TXIFG flag
		while (UCB0STAT & UCBUSY); // wait for SPI TX/RX to finish
	}
	//__delay_cycles(100);
	spi_dev->chip_release(cs_arg);
	return 0;
#endif

#ifdef __MSP430_HAS_USI__
	int bits_sent = 0;
	SPI_set_mode(spi_dev->spi_mode);
	spi_dev->chip_select(cs_arg);
	int bits_to_send = 0;
	while (bits_sent < spi_dev->bits) {
		bits_to_send = (((spi_dev->bits)-bits_sent) >= 16)?16:((spi_dev->bits)-bits_sent);
		USISR = value >> bits_sent;
		USICNT = (USI16B | bits_to_send);
		while (!(USICTL1&USIIFG));
		bits_sent += bits_to_send;
	}
	__delay_cycles(2);
	spi_dev->chip_release(cs_arg);
	return 0;
#endif
}
