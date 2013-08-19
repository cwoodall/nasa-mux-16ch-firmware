/*
 * dac7512.c
 *
 *  Created on: Jun 21, 2013
 *      Author: Christopher Woodall
 */
#include <stdint.h>
#include "spi/spi.h"
#include "spi/dac7512.h"
#include <msp430.h>


#define ROW_DATA_PIN   BIT3
#define COL_DATA_PIN   BIT0
#define MR_n_PIN       BIT2
#define SHIFT_LOAD_PIN BIT3
#define SHIFT_CLK_PIN  BIT1
#define SET_PIN(din, pin, pout) (din)?(pout |= (din << pin)):(pout &= ~(dout << pin))

void DAC7512_shiftreg_init() {
	// Setup Shift Register
	P1SEL &= ~ROW_DATA_PIN;
	P1DIR |= ROW_DATA_PIN;
	P1OUT |= ROW_DATA_PIN;
	P2DIR |= COL_DATA_PIN | MR_n_PIN | SHIFT_LOAD_PIN | SHIFT_CLK_PIN;
	P2OUT |= COL_DATA_PIN | MR_n_PIN;
	P2OUT &= ~(SHIFT_LOAD_PIN | SHIFT_CLK_PIN);
}

void DAC7512_shiftreg_load(uint8_t row_d, uint8_t col_d) {
	uint8_t i = 0;
	P2OUT |= MR_n_PIN;

	P2OUT &= ~SHIFT_LOAD_PIN;
	for (i = 0; i < 8; i++) {
		P2OUT &= ~SHIFT_CLK_PIN;
		if (row_d & 0x80) {
			P1OUT |= ROW_DATA_PIN;
		} else {
			P1OUT &= ~ROW_DATA_PIN;
		}

		if (col_d & 0x80) {
			P2OUT |= COL_DATA_PIN;
		} else {
			P2OUT &= ~COL_DATA_PIN;
		}
		P2OUT |= SHIFT_CLK_PIN;
		col_d <<= 1;
		row_d <<= 1;
	}
	P2OUT |= SHIFT_LOAD_PIN;
	P2OUT &= ~MR_n_PIN;
	P2OUT &= ~SHIFT_LOAD_PIN;
	P2OUT &= ~SHIFT_CLK_PIN;

	P2OUT |= COL_DATA_PIN;
	P1OUT |= ROW_DATA_PIN;
}

void DAC7512_init(spi_device_t *dev) {
	DAC7512_shiftreg_init();
	// SETUP SPI DEVICE
	dev->bits = 16;
	dev->spi_mode = SPI_MODE_2;
	dev->delay_cycles = 10;
	dev->chip_select  = *DAC7512_chip_select;
	dev->chip_release = *DAC7512_chip_release;
	DAC7512_shiftreg_load(0xff,0xff);
}
void DAC7512_send(spi_device_t *dev, uint16_t data, uint8_t r, uint8_t c) {
	// Row is bits 16 downto 8 (where each bit represents a row)
	// Col is bits 7 downto 0 (where each bit represents a column)
	SPI_send(dev, data, (1<<(r+8)) | (1<<c));
}

int16_t DAC7512_chip_select(uint16_t cs_arg) {
	// ROW[15:8] | COL[7:0]
	DAC7512_shiftreg_load(~(cs_arg >> 8), ~(cs_arg & 0xff));
	return 0;
}

int16_t DAC7512_chip_release(uint16_t cs_arg) {
	DAC7512_shiftreg_load(0xff,0xff);
	return 0;
}
