#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "crc16.h"

int main() {
	uint16_t crc = 0;
	crc_init(&crc);
	
	const char *test = "Lammert123";
	for (int i = 0; test[i] != '\0'; i++) {
		crc_add_byte(&crc, test[i]);
	}
	printf("CRC of '%s' is: %x\n", test, crc);

	return 0;
}
