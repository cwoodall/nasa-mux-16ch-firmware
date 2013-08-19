/*
 * spi.h
 *
 *  Created on: Jun 21, 2013
 *      Author: Christopher Woodall
 */

#ifndef SPI_H_
#define SPI_H_
#include <msp430.h>
#include <stdint.h>

#if defined(__MSP430_HAS_USCI__) || defined(__MSP430_HAS_EUSCI_B0__)

#define SPI_CLOCK_DIV1   1
#define SPI_CLOCK_DIV2   2
#define SPI_CLOCK_DIV4   4
#define SPI_CLOCK_DIV8   8
#define SPI_CLOCK_DIV16  16
#define SPI_CLOCK_DIV32  32
#define SPI_CLOCK_DIV64  64
#define SPI_CLOCK_DIV128 128

/**
 * USCI flags for various the SPI MODEs
 */

#define SPI_MODE_0 (UCCKPH)			    /* CPOL=0 CPHA=0 */
#define SPI_MODE_1 (0)                 	/* CPOL=0 CPHA=1 */
#define SPI_MODE_2 (UCCKPL | UCCKPH)    /* CPOL=1 CPHA=0 */
#define SPI_MODE_3 (UCCKPL)			    /* CPOL=1 CPHA=1 */

#define SPI_MODE_MASK (UCCKPL | UCCKPH)


#elif defined(__MSP430_HAS_USI__)

#define SPI_CLOCK_DIV1   0
#define SPI_CLOCK_DIV2   USIDIV_1
#define SPI_CLOCK_DIV4   USIDIV_2
#define SPI_CLOCK_DIV8   USIDIV_3
#define SPI_CLOCK_DIV16  USIDIV_4
#define SPI_CLOCK_DIV32  USIDIV_5
#define SPI_CLOCK_DIV64  USIDIV_6
#define SPI_CLOCK_DIV128 USIDIV_7

#define SPI_MODE_0 0x0 // SPI_MODE_0: CPOL = 0, CPHA = 0
#define SPI_MODE_1 0x2 // SPI_MODE_1: CPOL = 0, CPHA = 1
#define SPI_MODE_2 0x4 // SPI_MODE_2: CPOL = 1, CPHA = 0
#define SPI_MODE_3 0x6 // SPI_MODE_3: CPOL = 1, CPHA = 1

#else
    #error "SPI not supported by hardware on this chip"
#endif

// Define location of PORT 1 SPI pins. Chip Select must be setup seperately.
#define PIN_SCLK   BIT5 //1.5
#define PIN_MOSI   BIT6 //1.6
#define PIN_MISO   BIT7 //1.7


#define SELECT(PIN)  P1OUT &= ~PIN
#define DESELECT(PIN)  P1OUT |= PIN


typedef struct spi_device {
	// What SPI mode does the device require
	uint8_t spi_mode;

	// How many bits to be sent
	uint8_t bits;

	// Number of cycles to delay
	uint8_t delay_cycles;

	// Function pointer to the function that gets called on chip select
	int16_t (*chip_select)(uint16_t);
	// Function pointer to the function that gets called when a chip is released
	int16_t (*chip_release)(uint16_t);
} spi_device_t ;


void SPI_init();
inline void SPI_set_mode(uint8_t spi_mode);
uint16_t SPI_send(spi_device_t *spi_dev, uint32_t value, uint16_t cs_arg);

#endif /* SPI_H_ */
