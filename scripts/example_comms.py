import serial
import time
print "starting"
with serial.Serial('COM6', 9600, timeout=1) as nasa_mux:
#    nasa_mux.write("\x81\xf0\xbf\x04\x82");
    a = [0x81,0xf0,0xbf,0x04,0x82]
    for i in a:
        nasa_mux.write(chr(i))
    a = ""
    while ( a != "\x82"):
        a = nasa_mux.read(1)
        print "%x" % ord(a),
    print
    a = [0x81,0x85,0x10,0x04, 0x01, 0x00, 0x00,0x82]
    for i in a:
        nasa_mux.write(chr(i))
    while ( a != "\x82"):
        a = nasa_mux.read(1)
        print "%x" % ord(a),

