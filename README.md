# NASA MUX 16 Channel Firmware

## Serial Control Protocol

The serial control interface is a basic Rx/Tx UART with the following settings:

- Baudrate: 9600
- Data Bits: 8
- Stop Bits: 1
- Flow Control: None

The serial protocol is inspired by [Modbus][modbus]. Do __not__ follow the 
Modbus specification, instead follow the one layed down here.

The basic packet format is as follows in Table 1:

<center>Table 1</center>
| Name  | Bytes | Description                                                  |
|-------|-------|--------------------------------------------------------------|
| START |     1 | The start byte is the FRAME magic byte (0x81)                |
| CMD   |     1 | The command to be executed (ACK, WR_REG, ERR, etc)           |
| DATA  |     n |                                                              |
| CRC   |     2 | The CRC is a 16-bit CRC which uses the CRC16-IBM polynomial. the CRC is sent with the low byte first.                                |
| END   |     1 | The end byte is the the same as the start byte (0x81)        |


[modbus]: #
