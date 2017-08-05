#define STATION

const char* AP_ssid = "ESP8266 Access Point"; // The name of the Wi-Fi network that will be created
const char* AP_password = "thereisnospoon";   // The password required to connect to it, leave blank for an open network

const char* OTAName = "ESP8266";           // A name and a password for the OTA service
const char* OTAPassword = "espetss";

const char* WiFiHostname = "esp8266";      // Host name on the network
const char* mdnsName = "esp8266";          // Domain name for the mDNS responder

#include "Networkinit.hpp"
#include "OTA.hpp"
#include "Server.hpp"

const uint8_t outputs[] = { 2, 4, 5, 12, 13, 14, 15, 16 };
const size_t  nb_outputs = sizeof(outputs);

/*__________________________________________________________SETUP__________________________________________________________*/

void setup() {

  Serial.begin(115200);        // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println("\r\n");

  startOutputs();              // Set all LED pins to outputs

  startWiFi();                 // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection

  startOTA();                  // Start the OTA service

  startSPIFFS();               // Start the SPIFFS and list all contents

  startServer();               // Start a HTTP server with a file read handler and an upload handler

  server.on("/output",  HTTP_POST, setOutput);
  server.on("/output",  HTTP_GET, getOutput);

}

/*__________________________________________________________LOOP__________________________________________________________*/

void loop() {
  server.handleClient();                      // run the server
  ArduinoOTA.handle();                        // listen for OTA events
  printIP();
  printStations();
}

/*__________________________________________________________SETUP_FUNCTIONS__________________________________________________________*/

void startOutputs() {  // Set all LED pins to outputs
  for (uint8_t i = 0; i < nb_outputs; i++) {
    pinMode(outputs[i], OUTPUT);
  }
}

/*__________________________________________________________SERVER_HANDLERS__________________________________________________________*/

void setOutput() {
  if ( ! server.hasArg("output") || ! server.hasArg("state")
       || server.arg("output") == NULL) {                                 // If the POST request doesn't have "output" and "state" data
    server.send(400, "text/plain", "400: Invalid Request");               // The request is invalid, so send HTTP status 400
    return;
  }
  char state_char = server.arg("state").c_str()[0];  // the first character of the state
  if ( state_char != '1' &&  state_char != '0') {
    server.send(400, "text/plain", "400: Invalid Request");               // The request is invalid, so send HTTP status 400
    return;
  }
  bool state = state_char - '0';
  unsigned int output = server.arg("output").toInt();
  if (output >= nb_outputs) {
    server.send(404, "text/plain", "404: Output not found");              // The request is invalid, because the output doesn't exist
    return;
  }
  digitalWrite(outputs[output], state);                                   // Set the selected output accordingly
  server.send(200);
}

void getOutput() {
  char state_str[nb_outputs * 2 + 1 + 1]; // one digit + 1 comma for each output, last one doesn't have a comma but a ']', then one '[' and a NULL terminator
  state_str[0] = '[';                      // Get the state of the selected output
  for (uint8_t i = 0; i < nb_outputs - 1; i++) {
    state_str[2 * i + 1] = digitalRead(outputs[i]) + '0';
    state_str[2 * i + 2] = ',';
  }
  state_str[nb_outputs * 2 - 1] = digitalRead(outputs[nb_outputs - 1]) + '0';
  state_str[nb_outputs * 2] = ']';
  state_str[nb_outputs * 2 + 1] = '\0';
  server.send(200, "application/json", state_str);                        // Send it to the client
  Serial.println(ESP.getFreeHeap());
}
