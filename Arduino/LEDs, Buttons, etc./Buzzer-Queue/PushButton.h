#ifndef PUSHBUTTON_H_
#define PUSHBUTTON_H_

class PushButton
{
  public:

    PushButton(uint8_t pin) : pin(pin) {
      pinMode(pin, INPUT_PULLUP);
    };
    bool isPressed()
    {
      bool pressed = false;
      bool state = digitalRead(pin);
      int8_t stateChange = state - previousState;

      if (stateChange == falling) { // Button is pressed
        if (millis() - previousBounceTime > debounceTime) {
          pressed = true;
          previousBounceTime = millis();
        }
      }
      if (stateChange == rising) { // Button is released or bounces
        previousBounceTime = millis();
      }

      previousState = state;
      return pressed;
    };
  private:
    uint8_t pin;
    bool previousState = HIGH;
    unsigned long previousBounceTime = 0;

    const unsigned long debounceTime = 25;
    const int8_t rising = HIGH - LOW;
    const int8_t falling = LOW - HIGH;
};



#endif // PUSHBUTTON_H_
