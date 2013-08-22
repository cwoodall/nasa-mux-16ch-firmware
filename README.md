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
| START |     1 | The start byte is the FRAME magic byte (0x81)                |
| CMD   |     1 | The command to be executed (ACK, WR\_REG, ERR, etc)          |
| DATA  |     n | The length _n_ and the actual content of data is determined by CMD |
| CRC   |     2 | The CRC is a 16-bit CRC which uses the CRC16-IBM polynomial. the CRC is sent with the low byte first. |
| END   |     1 | The end byte is the the same as the start byte (0x81)        |

There are _magic_ bytes which effect the packets:

| Name     | Magic Byte | Description                                          |
|----------|------------|------------------------------------------------------|
| _FRAME_  |       0x81 | Indicates the start and the end of a packet, unless escaped. |
| _ESCAPE_ |       0x80 | Escapes the _FRAME_ and _ESCAPE_ magic bytes. When used it deactivates their magical properties. |

### Commands

__FIXME__: Insert Command Table for quick reference.

#### ~~ACK: Acknowledge (0x82) [NOT IMPLEMENTED]~~

ACK is just an acknowledgement, ACK should only be sent as a response. 

Data Format (n = 0): No data packet, just sends the command.

#### ~~ERR: Error (0x83) [NOT IMPLEMENTED]~~

ERR returns an error 
Data Format (n = 1):
 
| Name | Bytes | Description              |
| Type | 1     | Describes the error type |

Error Types:

| Name | Byte | Description   |
| GEN  | 0x00 | General Error |

#### WR_REG: Write Register (0x84)
#### ~~RD_REG: Read Register  (0x85) [NOT IMPLEMENTED]~~
#### ~~WR_BLOCK: Write Block (0x86) [NOT IMPLEMENTED]~~
#### ~~RD_BLOCK: READ Blcok (0x87) [ NOT IMPLEMENTED]~~
NOT IMPLEMENTED YET
[modbus]: #
