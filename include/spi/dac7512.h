/*
 * dac7512.h
 *
 *  Created on: Jun 21, 2013
 *      Author: Admin
 */

#ifndef DAC7512_H_
#define DAC7512_H_
#include <msp430.h>
#include <stdint.h>
#include "spi/spi.h"

void DAC7512_shiftreg_init();
void DAC7512_shiftreg_load(uint8_t row_d, uint8_t col_d);

void DAC7512_init(spi_device_t *dev);
void DAC7512_send(spi_device_t *dev, uint16_t data, uint8_t r, uint8_t c);

int16_t DAC7512_chip_select(uint16_t cs_arg);

int16_t DAC7512_chip_release(uint16_t cs_arg);


#endif /* DAC7512_H_ */
