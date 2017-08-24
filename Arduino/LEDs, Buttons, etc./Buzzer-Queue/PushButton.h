#ifndef PUSHBUTTON_H_
#define PUSHBUTTON_H_

const unsigned long debounceTime = 25;
const int8_t rising = HIGH - LOW;
const int8_t falling = LOW - HIGH;

struct PushButton
{
  PushButton(uint8_t pin) : pin(pin) {
    pinMode(pin, INPUT_PULLUP);
  };
  uint8_t pin;
  bool previousState = HIGH;
  unsigned long previousBounceTime = 0;
};

bool buttonPressed(PushButton &button)
{
  bool pressed = false;
  bool state = digitalRead(button.pin);
  int8_t stateChange = state - button.previousState;

  if (stateChange == falling) { // Button is pressed
    if (millis() - button.previousBounceTime > debounceTime) {
      pressed = true;
      button.previousBounceTime = millis();
    }
  }
  if (stateChange == rising) { // Button is released or bounces
    button.previousBounceTime = millis();
  }

  button.previousState = state;
  return pressed;
}

#endif // PUSHBUTTON_H_
