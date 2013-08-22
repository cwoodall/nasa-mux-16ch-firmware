#ifndef __CRC16_H__
#define __CRC16_H__
#include <stdint.h>

// The CRC table for the CRC16-IBM polynomial
extern const uint16_t crc16_ibm_table[256];
#define CRC_TABLE crc16_ibm_table
#define CRC_INIT_VALUE 0xffff
#define CRC_FINAL_VALUE 0x0000


/**
 * Initializes the 16 bit wide crc to the CRC_INIT_VALUE value. Use this to 
 * reset your crc polynomial.
 *
 * Parameters:
 *   uint16_t  *crc  Pointer to the crc variable, which is getting initialized 
 *                   to CRC_INIT_VALUE
 */
void crc_init(uint16_t *crc);


/**
 * Takes the 16-bit CRC polynomial calculated so far and calculates the crc
 * value with the next byte. This uses a 256 byte table calculated from the
 * CRC16-IBM polynomial
 *
 * @parameter  uint16_t  *crc  pointer to the crc value thus far.
 * @parameter  uint8_t  byte  next byte of data in the datastream.
 * @date 2013-08-22
 * @author Chris J. Woodall <cwoodall@bu.edu>
 */
void crc_add_byte(uint16_t *crc, uint8_t byte);

/**
 * Checks the crc polynomial to see if it is the CRC_FINAL_VALUE.
 */
char crc_check(uint16_t crc);

#endif
