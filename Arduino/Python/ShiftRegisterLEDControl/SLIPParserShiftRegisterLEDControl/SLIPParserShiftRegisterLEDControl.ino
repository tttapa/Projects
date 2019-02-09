struct SLIPParserCallbacks {
  virtual ~SLIPParserCallbacks() = default;
  virtual void onPacketEnd() = 0;
  virtual void onDataByteReceived(uint8_t data) = 0;
};

class SLIPParser {
  private:
    /* SLIP special character codes */
    static constexpr uint8_t END     = 0300;  // indicates end of packet
    static constexpr uint8_t ESC     = 0333;  // indicates byte stuffing
    static constexpr uint8_t ESC_END = 0334;  // ESC ESC_END means END data byte
    static constexpr uint8_t ESC_ESC = 0335;  // ESC ESC_ESC means ESC data byte

    bool ESC_received = false;
    SLIPParserCallbacks *callbacks = nullptr;

    void onPacketEnd() {
      if (callbacks)
        callbacks->onPacketEnd();
    }
    void onDataByteReceived(uint8_t data) {
      if (callbacks)
        callbacks->onDataByteReceived(data);
    }

  public:
    SLIPParser() = default;
    SLIPParser(SLIPParserCallbacks &callbacks) : callbacks(&callbacks) {}
    SLIPParser(SLIPParserCallbacks *callbacks) : callbacks(callbacks) {}
    
    void parse(uint8_t c) {
      if (ESC_received) {
        switch (c) {
          case ESC_END: c = END; break;
          case ESC_ESC: c = ESC; break;
          default: ; // protocol violation, just add data byte anyway
        }
        onDataByteReceived(c);
        ESC_received = false;
      } else {
        switch (c) {
          case END: onPacketEnd(); break;
          case ESC: ESC_received = true; break;
          default: onDataByteReceived(c);
        }
      }
    }

    void setCallbacks(SLIPParserCallbacks &callbacks) {
      this->callbacks = &callbacks;
    }
    void setCallbacks(SLIPParserCallbacks *callbacks) {
      this->callbacks = callbacks;
    }
};

// -------------------------------------------------------------------------- //

#include <SPI.h>

const uint8_t numberOfShiftRegisters = 3;
const SPISettings settings = {8000000, MSBFIRST, SPI_MODE0};

class ShiftRegCallbacks : public SLIPParserCallbacks {
  public: // TODO
    const uint8_t latchPin;

    void onPacketEnd() override {
      digitalWrite(latchPin, HIGH);
      digitalWrite(latchPin, LOW);
    }
    void onDataByteReceived(uint8_t data) override {
      SPI.beginTransaction(settings);
      SPI.transfer(data);
      SPI.endTransaction();
    }

  public:
    ShiftRegCallbacks(uint8_t latchPin) : latchPin(latchPin) {}
    void begin() {
      pinMode(latchPin, OUTPUT);
      digitalWrite(latchPin, LOW);
    }

    void zeros(uint8_t numberOfShiftRegisters) {
      SPI.beginTransaction(settings);
      while (numberOfShiftRegisters --> 0)
        SPI.transfer(0);
      SPI.endTransaction();
      digitalWrite(latchPin, HIGH);
      digitalWrite(latchPin, LOW);
    }
};

ShiftRegCallbacks shiftReg = {SS};
SLIPParser parser = {shiftReg};

void setup() {
  Serial.begin(1000000);
  SPI.begin();
  shiftReg.begin();
  shiftReg.zeros(numberOfShiftRegisters);
}

void loop() {
  if (Serial.available())
    parser.parse(Serial.read());
}
