/**
 * ad5504.c : init, send, chip release and chip_select code for the
 * 			  [AD5504 datasheet][1]
 *
 * [1]: http://www.analog.com/static/imported-files/data_sheets/AD5504.pdf
 *
 *  Created on: Jun 21, 2013
 *      Author: Chris Woodall
 */
#include "spi/spi.h"
#include "spi/ad5504.h"
#include <msp430.h>
#include <stdint.h>

extern const uint8_t ad5504_cs_pins[] = {0x10, 0x20, 0x40, 0x80};
// ad5504_addresses.
//   0 | CS BIT4 | AD5504_ADDRESS_A | 0x00
//   0 | cs
extern const uint16_t ad5504_addresses[] = {
	0x0100 | AD5504_CHA, 0x0100 | AD5504_CHB, 0x0100 | AD5504_CHC, 0x0100 | AD5504_CHD,
	0x0200 | AD5504_CHA, 0x0200 | AD5504_CHB, 0x0200 | AD5504_CHC, 0x0200 | AD5504_CHD,
	0x0400 | AD5504_CHA, 0x0400 | AD5504_CHB, 0x0400 | AD5504_CHC, 0x0400 | AD5504_CHD,
	0x0800 | AD5504_CHA, 0x0800 | AD5504_CHB, 0x0800 | AD5504_CHC, 0x0800 | AD5504_CHD
};

/**
 * Initialize a spi_device_t struct for the AD5504. These settings are taken from the [datasheet][1]
 */
void AD5504_init(spi_device_t *dev) {


	P2DIR |= 0xf0; // Setup the chip select pins.
	P2OUT &= ~(0xf0);

	dev->bits = 16; // According to datasheet messages to the AD5504 are 16 bits long
	dev->spi_mode = SPI_MODE_0; // According to the 5504 datasheet it looks like SPI mode 0.
								// Tested and verified by Christopher Woodall on August 2, 2013
	dev->delay_cycles = 10; 	// Currently unused
	dev->chip_select = *AD5504_chip_select;   // Set the chip select function pointer up
	dev->chip_release = *AD5504_chip_release; // Setup the chip release function pointer.
}

/**
 * Send function for the AD5504 which formats the 16, bit messages for the AD5504.
 * Message format is as follows:
 * +------++-----+--------------+-------------+
 * | Bits || 15  | 14 downto 12 | 11 downto 0 |
 * +======++=====+==============+=============+
 * | DOUT || R_W | Address 		| Data        |
 * +------++-----+--------------+-------------+
 */
int16_t AD5504_send(spi_device_t *dev, uint8_t address, uint16_t value, uint8_t read_write, uint8_t chip_address) {
	SPI_send(dev, ((read_write | address) << 12) | (value & 0x0FFF), chip_address);
	return 0;
}

/**
 * Select and AD5504.
 * AD5504 Chip Selects are on P2.4, P2.5, P2.6 and P2.7:
 *   - cs_arg should be 1, 2, 4 or 8 and then it will be shifted to the left by 4.
 *
 */
int16_t AD5504_chip_select(uint16_t cs_arg) {
	if ( (cs_arg == 0x1) || (cs_arg == 0x2) || (cs_arg == 0x4) || (cs_arg == 0x8)) {
		AD5504_SELECT(cs_arg<<4);
		return 0;
	} else {
		return -1; // Error state return
	}
}

/**
 * Deselect chip.
 */
int16_t AD5504_chip_release(uint16_t cs_arg) {
	if ( (cs_arg == 0x1) || (cs_arg == 0x2) || (cs_arg == 0x4) || (cs_arg == 0x8)) {
		AD5504_DESELECT(cs_arg<<4);
		return 0;
	} else {
		return -1; // Error state return
	}
}
