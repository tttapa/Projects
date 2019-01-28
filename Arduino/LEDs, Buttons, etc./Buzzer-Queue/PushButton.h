#pragma once

class PushButton {
  public:
    PushButton(uint8_t pin)    // Constructor (executes when a PushButton object is created)
      : pin(pin) {}                // remember the push button pin

    void begin() {
      pinMode(pin, INPUT_PULLUP); // enable the internal pull-up resistor
    }
    
    bool isPressed() { // read the button state check if the button has been pressed, debounce the button as well
      bool pressed = false;
      bool state = digitalRead(pin);               // read the button's state
      int8_t stateChange = state - previousState;  // calculate the state change since last time

      if (stateChange == falling) // If the button is pressed (went from high to low)
        if (millis() - previousBounceTime > debounceTime) // check if the time since the last bounce is higher than the threshold
          pressed = true; // the button is pressed
      if (stateChange == rising) // if the button is released or bounces
        previousBounceTime = millis(); // remember when this happened

      previousState = state; // remember the current state
      return pressed; // return true if the button was pressed and didn't bounce
    }
    
  private:
    uint8_t pin;
    bool previousState = HIGH;
    unsigned long previousBounceTime = 0;

    const unsigned long debounceTime = 25;
    const int8_t rising = HIGH - LOW;
    const int8_t falling = LOW - HIGH;
};
