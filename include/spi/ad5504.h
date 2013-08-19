/**
 * ad5504.h
 *
 *  Created on: Jun 21, 2013
 *      Author: Christopher Woodall
 */

#ifndef AD5504_H_
#define AD5504_H_
#include <msp430.h>
#include <stdint.h>
#include "spi/spi.h"


#define AD5504_CHA  0x1 // AD5504 Channel A Address
#define AD5504_CHB  0x2 // AD5504 Channel B Address
#define AD5504_CHC  0x3 // AD5504 Channel C Address
#define AD5504_CHD  0x4 // AD5504 Channel D Address
#define AD5504_ALL  0x5 // AD5504 All Channels Address
#define AD5504_CTRL 0x7 // AD5504 Control Address

#define AD5504_CHA_ON 0x04
#define AD5504_CHB_ON 0x08
#define AD5504_CHC_ON 0x10
#define AD5504_CHD_ON 0x20

#define AD5504_READ  0x8 // AD5504 read bit.
#define AD5504_WRITE 0x0 // AD5504 write bit

#define AD5504_SELECT(PIN)    P2OUT &= ~PIN
#define AD5504_DESELECT(PIN)  P2OUT |= PIN

extern const uint8_t ad5504_cs_pins[4];
extern const uint16_t ad5504_addresses[16];

void AD5504_init(spi_device_t *dev);
int16_t AD5504_send(spi_device_t *dev, uint8_t address, uint16_t value, uint8_t read_write, uint8_t chip_address);

int16_t AD5504_chip_select(uint16_t cs_arg);
int16_t AD5504_chip_release(uint16_t cs_arg);

//extern spi_device_t ad5504_device;

#endif /* AD5504_H_ */
