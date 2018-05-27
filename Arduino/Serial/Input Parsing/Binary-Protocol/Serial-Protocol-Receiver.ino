#include "Serial_Protocol.h"

void setup() {
  Serial.begin(115200);
  while (!Serial);
}

void loop() {
  uint16_t value;
  message_type type;
  if (receive(value, type)) {
    Serial.print(value);
    Serial.print('\t');
    Serial.println(type);
  }
}
