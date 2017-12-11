class PushButton
{
  public:
    PushButton(uint8_t pin) // Constructor (executes when a PushButton object is created)
      : pin(pin) { // remember the push button pin
      pinMode(pin, INPUT_PULLUP); // enable the internal pull-up resistor
    };
    bool isPressed() // read the button state check if the button has been pressed, debounce the button as well
    {
      bool pressed = false;
      bool state = digitalRead(pin);               // read the button's state
      int8_t stateChange = state - previousState;  // calculate the state change since last time

      if (stateChange == falling) { // If the button is pressed (went from high to low)
        if (millis() - previousBounceTime > debounceTime) { // check if the time since the last bounce is higher than the threshold
          pressed = true; // the button is pressed
        }
      }
      if (stateChange == rising) { // if the button is released or bounces
        previousBounceTime = millis(); // remember when this happened
      }

      previousState = state; // remember the current state
      return pressed; // return true if the button was pressed and didn't bounce
    };
  private:
    uint8_t pin;
    bool previousState = HIGH;
    unsigned long previousBounceTime = 0;

    const static unsigned long debounceTime = 25;
    const static int8_t rising = HIGH - LOW;
    const static int8_t falling = LOW - HIGH;
};

// -------------------------------------------------------------------------------------------------------------------------------- //

PushButton button = { 2 };  // Create a new PushButton object on pin 2

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);  // Set the built-in LED (pin 13) to output mode
}

void loop() {
    static bool LEDstate = LOW;
    if (button.isPressed()) {  // If the button is pressed
        LEDstate = !LEDstate;  // Flip the state of the LED
        digitalWrite(LED_BUILTIN, LEDstate);  // Write the new state to the LED
    }
}
