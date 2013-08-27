# NASA MUX 16 Channel Firmware

## Serial Control Protocol

The serial control interface is a basic Rx/Tx UART with the following settings:

- Baudrate: 9600
- Data Bits: 8
- Stop Bits: 1
- Flow Control: None
- Format: Bytes (as apposed to ASCII)

The serial protocol is inspired by [Modbus][modbus]. Do __not__ follow the 
Modbus specification, instead follow the one layed down here.

The basic packet format is as follows in Table 1:

| Name  | Bytes | Description                                                  |
|-------|-------|--------------------------------------------------------------|
| START |     1 | The start byte is the FRAME\_START magic byte (0x81)                |
| CMD   |     1 | The command to be executed (ACK, WR\_REG, ERR, etc)          |
| DATA  |     n | The length _n_ and the actual content of data is determined by CMD |
| CRC   |     2 | The CRC is a 16-bit CRC which uses the CRC16-IBM polynomial. the CRC is sent with the low byte first. |
| END   |     1 | The end byte is the FRAME\_END magic byte (0x82)        |

There are _magic_ bytes which effect the packets:

| Name           | Magic Byte | Description                                          |
|----------------|------------|------------------------------------------------------|
| _ESCAPE_       |       0x80 | Escapes magic bytes to deactivates their magical properties. |
| _FRAME\_START_ |       0x81 | Indicates the start and the end of a packet, unless escaped. |
| _FRAME\_END_   |       0x82 | Indicates the start and the end of a packet, unless escaped. |

The _magic_ bytes must be escaped if they must occur in data. For example, if I wanted to send a 0x81 in the data then I will need to escape it and send two bytes instead of one.
I would then send `0x80 0x81`.

Magic bytes, unless they are escaped, are not included in the CRC calculation.
### Commands

__FIXME__: Insert Command Table for quick reference.

#### ACK: Acknowledge (0x83)

_ACK_ is just an acknowledgement, _ACK_ should only be sent as a response. 
The contents of _ACK_ depend on the command sent. For example, _ACK_ will
have a data length of 0 for _WR\_REG_ or _WR\_BLOCK_ commands. However,
For _RD\_REG_ and _RD\_BLOCK_ commands (read commands in general) it will
hold the requested information in the data section.

Data Format (n = ?): Data packet in _ACK_ specified by command.

Response: _ACK_ with n = 0.

#### ERR: Error (0x84)

ERR returns an error 
Data Format (n = 1):
 
| Name | Bytes | Description              |
|------|-------|--------------------------|
| Type | 1     | Describes the error type |

Error Types:

| Name         | Byte | Description        |
|--------------|------|--------------------|
| GEN          | 0x00 | General Error      |
| CRC          | 0x01 | CRC Not Valid      |
| BAD_PACKET   | 0x02 | Packet has a bad format at some point |
| BAD_ADDRESS  | 0x03 | Packet has an invalid address |
| FRAME        | 0x04 | Unexpected or spurious _FRAME\_START_ byte |

Response: No response expected to _ERR_.

#### WR_REG: Write Register (0x85)

Data Format (n = 3):

|Name     | Bytes | Description         |
|---------|-------|---------------------|
| Address | 1     | Address MSB First   |
| Data    | 2     | Data sent MSB first |

Response: ACK with data length = 0.

#### READ_REG: Read Register  (0x86)

Data Format (n = 1):

|Name     | Bytes | Description         |
|---------|-------|---------------------|
| Address | 1     | Address MSB First   |

Response: ACK with data length = 2, which contains the 16 bits in the register. MSB first.

#### ~~WR_BLOCK: Write Block (0x86) [NOT IMPLEMENTED]~~
#### ~~READ_BLOCK: Read Block (0x87) [NOT IMPLEMENTED]~~

#### DISABLE_CRC: Disable CRC Checks on the MSP430 (0xf0) [USE WITH CAUTION]

__CAUTION__ By disabling CRC you still need to send the CRC bytes, they just are not checked. The suggestion is that you send the CRC 0x0000 or 0xDEAD.

Data Format (n = 0): No data sent

Response: ACK with data length = 2, which is 0xDEAD. MSB first.

Example Packet:
| __Time__   |  0   |   1  |  2   |  3   |  4 
|------------|------|------|------|------|------
| __Packet__ | 0x81 | 0xf0 | 0xBF | 0x04 | 0x82
		
#### ENABLE_CRC: Enables CRC Checks on the MSP430 (0xf1) [DEFAULT]

Data Format (n = 0): No data sent

Response: ACK with data length = 2, which is 0xBEEF. MSB first.

### CRC 

The CRC used is the CRC-16 used by MODBUS and implemented the same way. While all data is sent Most Significant Byte first the CRC must be sent Least Signigicant Byte First. This is do to the way that the CRC algorithm is computed on the MSP430. You can type in a message [here][lammert] and see what the CRC is (look at `CRC-16 (Modbus)`).

Included here is some example CRC code for using the CRC-16 table I implemented. 
Code is written in C.

Library (`src/crc16/crc16.c` and `include/crc16/crc16.h`)

```c
#include "crc16/crc16.h"

const uint16_t crc16_ibm_table[] = {
		0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
		0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
		0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
		0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
		0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
		0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
		0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
		0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
		0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
		0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
		0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
		0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
		0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
		0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
		0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
		0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
		0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
		0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
		0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
		0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
		0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
		0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
		0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
		0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
		0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
		0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
		0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
		0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
		0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
		0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
		0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
		0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040 };

void crc_init(uint16_t *crc) {
	*crc = CRC_INIT_VALUE;
}

void crc_add_byte(uint16_t *crc, uint8_t byte) {
	uint8_t temp = (*crc) ^ byte;
	(*crc) >>= 8;
	(*crc) ^= CRC_TABLE[temp];
}

char crc_check(uint16_t crc) {
	return (crc == CRC_FINAL_VALUE);
}
```

Example Use (Checking CRC):

```c
#include <stdint.h>
#include "crc16/crc16.h"
#define LEN 6

// When this message is run through the CRC algorithm it should result in a 0.
uint8_t message[LEN] = {0x85, 0x00, 0x00, 0x00, 0x29, 0x28};
uint16_t crc;
crc_init(&crc)

//  Initialize the crc
for (int i = 0; i < LEN; i++) {
  crc_add_byte(&crc, message[i]);
}

if (crc == 0) {
  printf("Success\n");
} else {
  printf("Failure\n");
}
```

Example Use (Generating CRC):

```c
#include <stdint.h>
#include "crc16/crc16.h"

#define LEN 4
// When this message is run through the CRC algorithm it should result in a 0x2829.
uint8_t message[LEN] = {0x85, 0x00, 0x00, 0x00};
uint16_t crc;

//  Initialize the crc
crc_init(&crc)

for (int i = 0; i < LEN; i++) {
  crc_add_byte(&crc, message[i]);
}

printf("CRC-16 (Modbus): 0x%04x\n", crc)
printf("Formatted to be sent: { 0x%02x, 0x%02x }", crc&0xff, crc>>8)
```

### Address Table

| Name          | Address   | Description 
|---------------|-----------|-------------
| Settings      | 0x00      | Settings Register: Bit 0 controls the Red LED on the MSP430
| AD5504 #1     | 0x10      | The bottom 12 bits control the value of AD5504 1  (R1C1)
| AD5504 #2     | 0x11      | The bottom 12 bits control the value of AD5504 2  (R1C2)  
| AD5504 #3     | 0x12      | The bottom 12 bits control the value of AD5504 3  (R1C3)  
| AD5504 #4     | 0x13      | The bottom 12 bits control the value of AD5504 4  (R1C4)  
| AD5504 #5     | 0x14      | The bottom 12 bits control the value of AD5504 5  (R2C1)  
| AD5504 #6     | 0x15      | The bottom 12 bits control the value of AD5504 6  (R2C2)  
| AD5504 #7     | 0x16      | The bottom 12 bits control the value of AD5504 7  (R2C3)  
| AD5504 #8     | 0x17      | The bottom 12 bits control the value of AD5504 8  (R2C4)  
| AD5504 #9     | 0x18      | The bottom 12 bits control the value of AD5504 9  (R3C1)  
| AD5504 #10    | 0x19      | The bottom 12 bits control the value of AD5504 10 (R3C2)  
| AD5504 #11    | 0x1a      | The bottom 12 bits control the value of AD5504 11 (R3C3)  
| AD5504 #12    | 0x1b      | The bottom 12 bits control the value of AD5504 12 (R3C4)  
| AD5504 #13    | 0x1c      | The bottom 12 bits control the value of AD5504 13 (R4C1)  
| AD5504 #14    | 0x1d      | The bottom 12 bits control the value of AD5504 14 (R4C2)  
| AD5504 #15    | 0x1e      | The bottom 12 bits control the value of AD5504 15 (R4C3)  
| AD5504 #16    | 0x1f      | The bottom 12 bits control the value of AD5504 16 (R4C4)  
| DAC7512 #1    | 0x20      | The bottom 12 bits control the value of DAC7512 1  (R1C1)
| DAC7512 #2    | 0x21      | The bottom 12 bits control the value of DAC7512 2  (R1C2)  
| DAC7512 #3    | 0x22      | The bottom 12 bits control the value of DAC7512 3  (R1C3)  
| DAC7512 #4    | 0x23      | The bottom 12 bits control the value of DAC7512 4  (R1C4)  
| DAC7512 #5    | 0x24      | The bottom 12 bits control the value of DAC7512 5  (R2C1)  
| DAC7512 #6    | 0x25      | The bottom 12 bits control the value of DAC7512 6  (R2C2)  
| DAC7512 #7    | 0x26      | The bottom 12 bits control the value of DAC7512 7  (R2C3)  
| DAC7512 #8    | 0x27      | The bottom 12 bits control the value of DAC7512 8  (R2C4)  
| DAC7512 #9    | 0x28      | The bottom 12 bits control the value of DAC7512 9  (R3C1)  
| DAC7512 #10   | 0x29      | The bottom 12 bits control the value of DAC7512 10 (R3C2)  
| DAC7512 #11   | 0x2a      | The bottom 12 bits control the value of DAC7512 11 (R3C3)  
| DAC7512 #12   | 0x2b      | The bottom 12 bits control the value of DAC7512 12 (R3C4)  
| DAC7512 #13   | 0x2c      | The bottom 12 bits control the value of DAC7512 13 (R4C1)  
| DAC7512 #14   | 0x2d      | The bottom 12 bits control the value of DAC7512 14 (R4C2)  
| DAC7512 #15   | 0x2e      | The bottom 12 bits control the value of DAC7512 15 (R4C3)  
| DAC7512 #16   | 0x2f      | The bottom 12 bits control the value of DAC7512 16 (R4C4)  

### Implementation Concerns

The current implementation on the MSP430 uses a state machine for processing which contains
3 state: _IDLE_, _MESSAGE_, _ESCAPE_. In the _IDLE_ state you are not currently in a packet and are awaiting to see a start of packet _FRAME\_START_ (0x81). In the _MESSAGE_ state you add the bytes you get to the CRC (except _ESCAPE_ bytes), when you see the _FRAME\_END_ you check the CRC (if CRC is enabled) and then process the packet on hand. If a spurious _FRAME\_START_ comes along you clear the CRC, clear the buffer and send a _FRAME\_ERROR_, you also try to sync into the next packet (currently problematic). If an _ESCAPE_ byte comes during the _MESSAGE_ state you don't CRC it or add it to the buffer and move to the _ESCAPE_ state which simply adds the next byte to the buffer and CRC regardless of its magical properties.


       +--------[FRAME_END (0x82)]-----------+
       |                                     |
       V                                     |
    +------+                             +-------+
	| IDLE | ---[FRAME_START (0x81)]---->|MESSAGE|<---------------+
	+------+                             +-------+                |
	                                         |                    |
                                      [ESCAPE (0x80)]             |
											 |                    |
									 		 V                    |
									    +--------+                |
										| ESCAPE |---[ANY BYTE]---+
										+--------+
[modbus]: #
[lammert]: http://www.lammertbies.nl/comm/info/crc-calculation.html
