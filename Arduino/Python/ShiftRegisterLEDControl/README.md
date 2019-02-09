## Python SLIP LED Control

This is an example for controlling a bunch of LEDs from a Python script, over 
the serial port.  
The LEDs in this example are driven by 74HC595 shift registers.  

### Connections
- SS:   74HC595 ST_CP
- MOSI: 74HC595 DS
- CLK:  74HC595 SH_CP

If you chain multiple shift registers together, the ST_CP and SH_CP lines are 
common to all chips, the data line only goes to the DS pin of the first chip, 
the Q7S pin (serial data out) of the first chip goes to the DS pin of the 
second, and so on.

I used 3 shift registers chained together, but you can use any number, just
change it in the Arduino code, and change the amount of data is being sent in
the Python script.

The output enable pins should be connected to ground.  

Remember to use series resistors with your LEDs, don't connect them to the 
outputs of the shift registers directly.

### Details
The serial data is encoded using the SLIP protocol described in
[RFC1055](https://tools.ietf.org/html/rfc1055).  
The Python script sends out bytes that are shifted out to the shift registers 
directly. When an `END` marker is received, the data is latched out to the 
output registers.

### Dependencies
You need to install the [pySerial](https://pypi.org/project/pyserial/) package. 

### Troubleshooting
You may have to change the serial port in the Python script to `COM1` or 
something similar if you're on Windows.