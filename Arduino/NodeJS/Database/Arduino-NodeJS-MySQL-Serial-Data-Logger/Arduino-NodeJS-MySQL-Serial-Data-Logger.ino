/**
 * Send the value of analog input A0 
 * over the Serial connection every ten seconds.
 */

const uint8_t sensorPin = A0;
const unsigned long interval = 10000; // 10,000 ms = 10 s

void setup() {
    Serial.begin(115200);
    Serial.println("Hello,");
    Serial.println("World!");
}

void loop() {
    static unsigned long previousMillis = 0;
    if (millis() - previousMillis >= interval) {
        Serial.println(analogRead(sensorPin));
        previousMillis += interval;
    }
}
