#!/usr/bin/env python3

import serial
from time import sleep

port = '/dev/ttyACM0'
baud = 1e6

# See RFC1055: Serial Line IP
# https://tools.ietf.org/html/rfc1055

# SLIP special character codes
END             = b'\300'   # indicates end of packet
ESC             = b'\333'   # indicates byte stuffing
ESC_END         = b'\334'   # ESC ESC_END means END data byte
ESC_ESC         = b'\335'   # ESC ESC_ESC means ESC data byte

def sendSLIP(port : serial.Serial, data : bytes):
    for byte in serial.iterbytes(data):
        if byte == END:
            port.write(ESC)
            port.write(ESC_END)
        elif byte == ESC:
            port.write(ESC)
            port.write(ESC_ESC)
        else:
            port.write(byte)
    port.write(END)

with serial.Serial(port=port, baudrate=baud) as port:
    sleep(2.1)
    while(True):
        data = bytes((0b10010010, 0b01001001, 0b00100100))
        sendSLIP(port, data)
        sleep(1)
        data = bytes((0b11011011, 0b01101101, 0b10110110))
        sendSLIP(port, data)
        sleep(1)
        data = bytes((0b01001001, 0b00100100, 0b10010010))
        sendSLIP(port, data)
        sleep(1)
        data = bytes((0b01101101, 0b10110110, 0b11011011))
        sendSLIP(port, data)
        sleep(1)
        data = bytes((0b00100100, 0b10010010, 0b01001001))
        sendSLIP(port, data)
        sleep(1)
        data = bytes((0b10110110, 0b11011011, 0b01101101))
        sendSLIP(port, data)
        sleep(1)
