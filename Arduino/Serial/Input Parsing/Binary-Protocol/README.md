# Binary Protocol
This is a simple protocol I've used for a health monitoring device.  
It can send 8 different messages, each with a 10-bit payload.

Messages consist of 2 bytes. To prevent framing errors, the first header byte has the msb (most significant bit) set to 1, and the following data byte has the msb set to 0.    
The format is as follows:  

| Byte 0 |     |     |     |     |     |     |     |     | 
|:-------|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| bit    |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  | 
| value  | *1* | *m* | *m* | *m* | *x* | *t* | *t* | *t* | 

| Byte 1 |     |     |     |     |     |     |     |     | 
|:-------|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:| 
| bit    |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  | 
| value  | *0* | *l* | *l* | *l* | *l* | *l* | *l* | *l* |

*`t`:* message type (3-bit)  
*`x`:* reserved for future use (1-bit)  
*`m`:* three most significant bits of the 10-bit value (3-bit)  
*`l`:* seven least significant bits of the 10-bit value (7-bit)  

#### Message types
`000`: ECG  
`001`: PPG red  
`010`: PPG IR  
`011`: Pressure A  
`100`: Pressure B  
`101`: Pressure C  
`110`: Pressure D  
`111`: Command  

#### Commands
`000`: Cancel panic  
`001`: Panic button  
`010`: LED off  
`011`: LED on  
`100`: Buzzer off  
`101`: Buzzer on  

### Implementation

```cpp
enum message_type {
  ECG         = 0b000,
  PPG_RED     = 0b001,
  PPG_IR      = 0b010,
  PRESSURE_A  = 0b011,
  PRESSURE_B  = 0b100,
  PRESSURE_C  = 0b101,
  PRESSURE_D  = 0b110,
  COMMAND     = 0b111
};

enum command_type : uint16_t {
  NO_PANIC    = 0b0000,
  PANIC       = 0b0001,
  LED_OFF     = 0b0010,
  LED_ON      = 0b0011,
  BUZZER_OFF  = 0b0100,
  BUZZER_ON   = 0b0101
};

void encode(uint8_t (&buffer)[2], uint16_t value, message_type type = COMMAND) {
  buffer[1] = value & 0b01111111;         // l
  buffer[0] = (value >> 3) & 0b01110000;  // m
  buffer[0] |= (type & 0b0111);           // t
  buffer[0] |= 0b10000000;                // set msb
}

void decode(uint8_t (&buffer)[2], uint16_t &value, message_type &type) {
  value = buffer[1] | ((buffer[0] & 0b01110000) << 3);
  type = static_cast<message_type>(buffer[0] & 0b0111);
}

void send(uint16_t value, message_type type = COMMAND) {
  uint8_t messageToSend[2];
  encode(messageToSend, value, type);
  Serial.write(messageToSend, sizeof(messageToSend));
}

bool receive(uint16_t &value, message_type &type) {
  if (!Serial.available())
    return false;
  static uint8_t messageReceived[2] = {};
  uint8_t data = Serial.read();
  if (data & 0b10000000) {  // If it's a header byte (first byte)
    messageReceived[0] = data;
    return false;
  } else if (messageReceived[0]) {  // If it's a data byte (second byte) and a first byte has been received
    messageReceived[1] = data;
    decode(messageReceived, value, type);
    messageReceived[0] = 0;
    return true;
  } else {
    return false;
  }
}
```
### Examples
[Serial-Protocol-Receiver](Serial-Protocol-Receiver.ino): Receives serial messages, decodes them, and displays the 10-bit value and message type.  
[Serial-Protocol-Sender](Serial-Protocol-Sender.ino): Sends encoded ECG messages over Serial with A0 values, as well as panic button messages when pin 2 is pulled low. 
