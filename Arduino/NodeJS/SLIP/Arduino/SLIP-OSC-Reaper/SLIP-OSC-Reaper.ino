/*
 * A simple OSC SLIP example, using the CNMAT/OSC library.
 * 
 * Connect a potentiometer to pin A0.
 * 
 * Connect the Arduino via USB and start the Node.js SLIP-to-UDP script.
 * Select the correct ports and IP address in Reaper.
 * 
 * When moving the potentiometer, the master volume should change.
 * When muting channel 1, the built-in LED on the Arduino should turn on.
 * 
 * 8-bit AVR Arduinos don't seem to have enough memory for receiving
 * OSC bundles using the CNMAT/OSC library (it reallocates on every 
 * byte it receives, probably resulting in heap fragmentation, causing
 * malloc to fail, crashing the entire program).
 * A simpler library with a static buffer could solve this problem.
 * 
 * https://github.com/tttapa/Projects/blob/master/Arduino/NodeJS/SLIP
 */

#include "EMA.hpp"
#include "Hysteresis.h"

#include <OSCBundle.h>
#include <OSCBoards.h>

#ifdef BOARD_HAS_USB_SERIAL
#include <SLIPEncodedUSBSerial.h>
SLIPEncodedUSBSerial SLIPSerial( thisBoardsSerialUSB );
#else
#include <SLIPEncodedSerial.h>
SLIPEncodedSerial SLIPSerial(Serial);
#endif

void setup() {
  SLIPSerial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
}

constexpr unsigned long faderInterval = 10; // Refresh the fader every 10 ms

/* Read the analog input A0, filter it, and send
 *  it over OSC as a float between 0 and 1 to 
 *  address "/master/volume".
 */
void sendMasterVolume() {
  static EMA<2, int16_t> ema;
  static Hysteresis hyst;
  static uint8_t prevValue = 0xFF;

  uint16_t raw = analogRead(A0);
  uint16_t filtered = ema.filter(raw);
  uint8_t value = hyst.getOutputLevel(filtered);
  if (value != prevValue) {
    OSCMessage msg("/master/volume");
    msg.add((float)value / 127.0);
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    msg.empty(); // free space occupied by message
    prevValue = value;
  }
}

/* When receiving an OSC message on address
 *  /track/x/mute, check the track, and turn
 *  on the built-in LED when track 1 is muted,
 *  or off when it is unmuted. 
 */
void muteHandler(OSCMessage &msg) {
  static char address[3] = {};
  if (msg.isFloat(0)) {
    bool status = msg.getFloat(0) > 0.0;
    msg.getAddress(address, 7, sizeof(address) / sizeof(*address) - 1);
    int track = atoi(address);
    if (track == 1) {
      digitalWrite(LED_BUILTIN, status);
    }
  }
}

void loop() {
  static unsigned long prevMillis = millis();
  if (millis() - prevMillis >= faderInterval) {
    sendMasterVolume();
    prevMillis += faderInterval;
  }

  static OSCBundle *bundleIN = new OSCBundle; // See https://github.com/CNMAT/OSC/issues/87
  bool eot =  SLIPSerial.endofPacket();
  while (SLIPSerial.available() && !eot) {
    uint8_t data = SLIPSerial.read();
    bundleIN->fill(data);
    eot =  SLIPSerial.endofPacket();
  }
  if (eot) {
    if (!bundleIN->hasError()) {
      bundleIN->dispatch("/track/*/mute", muteHandler);
    }
    delete bundleIN;
    bundleIN = new OSCBundle;
  }
}
