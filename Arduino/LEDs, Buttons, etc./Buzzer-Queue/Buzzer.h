#pragma once

class Buzzer {
  private:
    const static size_t bufferLen = 16;    // the number of beeps that can be saved in the buffer
    struct QueueEntry {
      unsigned int onDelay;
      unsigned int offDelay;
    } FIFO[bufferLen] = {};  // the actual buffer to save beeps (each beep has two values, a duration that it has to stay on, and a duration that it has to stay off)
  
    uint8_t pin;                           // the digital pin with a buzzer connected
    unsigned long offTime = 0;             // the moment the buzzer was turned off
    unsigned long onTime  = 0;             // the moment the buzzer was turned on
    unsigned int offDelay = 0;             // the duration that it should stay off
    unsigned int onDelay  = 0;             // the duration that it should stay on   (not really necessary, but it improves readability)
    size_t index = 0;                      // the index of the beep that is currently being played
    size_t writeIndex = 0;                 // the index where new beeps are added to the buffer
    size_t beepsInQueue = 0;               // the number of beeps that are in the queue (zero when the queue is empty)
    bool beeping = false;                  // the state of the buzzer (is it on or off)

    void beepEnable() {
      if (beepsInQueue == 0) // if there are no beeps in the queue, there's nothing to enable
        return;              // so exit the function

      digitalWrite(pin, HIGH);       // enable the buzzer
      beeping = true;                // remember that the buzzer is on
      onTime = millis();             // remember at what time the buzzer was turned on
      onDelay = FIFO[index].onDelay; // remember how long it should stay on
    }
    void beepDisable() {
      digitalWrite(pin, LOW);          // disable the buzzer
      beeping = false;                 // remember that the buzzer is off
      offTime = millis();              // remember at what time the buzzer was turned off
      offDelay = FIFO[index].offDelay; // remember how long it should stay off

      index = (index + 1) % bufferLen; // increment the index (or wrap around if the maximum index is reached)
      beepsInQueue--;                  // the beep is done, so it's no longer in the queue
    }

  public:
    Buzzer(uint8_t pin) // Constructor (executes when a Buzzer object is created)
      : pin(pin) {} // remember the buzzer pin
      
    void begin() {
      pinMode(pin, OUTPUT); // set the buzzer pin as an output
    }
    
    void refresh() {
      if (!beeping                // if the buzzer is off
          && (millis() - offTime) // and the time elapsed since the moment it was turned off
          >= offDelay) {          // is longer than the time it was to stay off ofter the previous beep:
        beepEnable();             // turn on the next beep
      }
      if (beeping                 // if the buzzer is on
          && (millis() - onTime)  // and the time elapsed since the moment it was turned on
          >= onDelay ) {          // is longer than the time it was to stay on:
        beepDisable();            // stop beeping
      }
    }

    void queueBeep(unsigned int onDelay = 100, unsigned int offDelay = 100) {
      if (beepsInQueue >= bufferLen) // if the buffer is full, we can't add new biep to the queue
        return;                      // so exit the function

      FIFO[writeIndex].onDelay = onDelay;  // write the parameters of the beep to the buffer
      FIFO[writeIndex].offDelay = offDelay;
      beepsInQueue++;                 // a beep was added to the queue, so increment the number of beeps in the queue
      writeIndex = (writeIndex + 1) % bufferLen; // increment the write index (or wrap around if the maximum index is reached)
    }
};
