/*
 * shiftreg.h
 *
 *  Created on: Aug 19, 2013
 *      Author: Admin
 */

#ifndef SHIFTREG_DEV_H_
#define SHIFTREG_DEV_H_

typedef struct shiftreg_device {
	// What SPI mode does the device require
	uint8_t reset_pin;
	uint8_t reset_port;
	uint8_t clock_pin;
	uint8_t data_pin;
	// How many bits to be sent
	uint8_t bits;

	// Number of cycles to delay
	uint8_t delay_cycles;

	// Function pointer to the function that gets called on chip select
	int16_t (*chip_select)(uint16_t);
	// Function pointer to the function that gets called when a chip is released
	int16_t (*chip_release)(uint16_t);
} shiftreg_device_t ;



#endif /* SHIFTREG_H_ */
