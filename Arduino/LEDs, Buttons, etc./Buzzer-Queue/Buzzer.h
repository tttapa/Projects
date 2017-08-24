#ifndef BUZZER_H_
#define BUZZER_H_

class Buzzer {
  private:
    uint8_t pin;
    const static size_t bufferLen = 16;
    unsigned int FIFO[bufferLen][2] = {};
    unsigned long offTime = 0;
    unsigned long onTime  = 0;
    unsigned int offDelay = 0;
    unsigned int onDelay  = 0; // you don't really need this, but it improves readability
    size_t index = 0;
    size_t writeIndex = 0;
    size_t beepsInQueue = 0;
    bool beeping = false;

    void beepEnable() {
      if (beepsInQueue == 0)
        return;
      beeping = true;
      digitalWrite(pin, HIGH);
      onTime = millis();
      onDelay = FIFO[index][0];
    }
    void beepDisable() {
      beeping = false;
      digitalWrite(pin, LOW);
      offTime = millis();
      offDelay = FIFO[index][1];
      index = (index + 1) % bufferLen;
      beepsInQueue--;
    }

  public:
    void refresh() {
      if (!beeping && (millis() - offTime >= offDelay)) {
        beepEnable();
      }
      if (beeping  && (millis() - onTime  >= onDelay )) {
        beepDisable();
      }
    }

    void queueBeep(unsigned int onDelay = 100, unsigned int offDelay = 100) {
      if (beepsInQueue >= bufferLen)
        return;
      FIFO[writeIndex][0] = onDelay;
      FIFO[writeIndex][1] = offDelay;
      beepsInQueue++;
      writeIndex = (writeIndex + 1) % bufferLen;
    }
    Buzzer(uint8_t pin) : pin(pin) {
      pinMode(pin, OUTPUT);
    }
};

#endif // BUZZER_H_
