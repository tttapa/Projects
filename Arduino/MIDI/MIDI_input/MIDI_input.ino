const uint8_t CONTROL_CHANGE = 0xB;
const uint8_t NOTE_OFF = 0x8;
const uint8_t NOTE_ON = 0x9;

struct MIDI_message {
  union {
    struct {
      uint8_t channel : 4;
      uint8_t message_type : 4;
    };
    uint8_t status;
  };
  uint8_t data1;
  uint8_t data2;
};

bool receiveMIDI(Stream &s, MIDI_message &m) { // Receives 3-Byte MIDI messages
  static uint8_t header = 0;
  static uint8_t data1 = 0b10000000;
  if (!s.available())
    return false;
  uint8_t newByte = s.read();
  if (newByte & 0b10000000) {   // Header byte received
    header = newByte;
    return false;
  }
  if (header & 0b10000000) {
    if (data1 == 0b10000000) {  // First data byte received
      data1 = newByte;
      return false;
    }
    m.status = header;          // Second data byte received
    m.data1 = data1;
    m.data2 = newByte;
    data1 = 0b10000000;
    return true;
  }
  return false;                 // Data byte without header received
}

void setup() {
  Serial.begin(31250);
}

void loop() {
  MIDI_message midimsg;
  if (receiveMIDI(Serial, midimsg)) {
    if (midimsg.message_type == CONTROL_CHANGE) {
      /*
       * Don't print ASCII to a MIDI device, obviously, 
       * just for demonstration purposes.
       * 
      Serial.print("Received MIDI Control Change message on channel ");
      Serial.print(midimsg.channel + 1);
      Serial.print(", controller #");
      Serial.print(midimsg.data1);
      Serial.print(", value = ");
      Serial.println(midimsg.data2);
      */
    } else if (midimsg.message_type == NOTE_ON) {
      /* handle note on event */
    } else if (midimsg.message_type == NOTE_OFF) {
      /* handle note off event */
    }
  }
}
