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
#### ~~RD_BLOCK: READ Blcok (0x87) [ NOT IMPLEMENTED]~~

### CRC 

### Address Table

| Name     | Address | Description | Permissions |
|----------|---------|-------------|-------------|
| Settings | 0x00    |
[modbus]: #
