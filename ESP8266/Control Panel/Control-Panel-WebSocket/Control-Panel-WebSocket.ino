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
// #include "WebSocket.hpp"

#include <WebSocketsServer.h>

WebSocketsServer webSocket = WebSocketsServer(81);


const uint8_t outputs[] = { 2, 4, 5, 12, 13, 14, 15, 16 };
const uint8_t  nb_outputs = sizeof(outputs);

char nb_outputs_str[4] = "#FF";  // "#FF" + null
char output_state_str[5] = "FF:S";  // "FF:S" + null

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

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  generate_nb_outputs_str();

}

/*__________________________________________________________LOOP__________________________________________________________*/

void loop() {
  server.handleClient();                      // run the server
  ArduinoOTA.handle();                        // listen for OTA events
  webSocket.loop();                           // run the WebSocket server
  printIP();
  printStations();
}

/*__________________________________________________________SETUP_FUNCTIONS__________________________________________________________*/

void startOutputs() {  // Set all LED pins to outputs
  for (uint8_t i = 0; i < nb_outputs; i++) {
    pinMode(outputs[i], OUTPUT);
  }
}

void generate_nb_outputs_str() {
  // nb_outputs_str[0] = '#';
  byte_to_str(&nb_outputs_str[1], nb_outputs);
}

void byte_to_str(char* buff, uint8_t val) {
  buff[0] = nibble_to_hex(val >> 4);
  buff[1] = nibble_to_hex(val);
  // buff[2] = '\0';
}

char nibble_to_hex(uint8_t nibble) {
  nibble &= 0xF;
  return nibble > 9 ? nibble - 10 + 'A' : nibble + '0';
}

/*__________________________________________________________WEBSOCKET__________________________________________________________*/

void webSocketEvent(uint8_t WS_client_num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", WS_client_num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(WS_client_num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", WS_client_num, ip[0], ip[1], ip[2], ip[3], payload);
        webSocket.sendTXT(WS_client_num, nb_outputs_str); // send the number of outputs, e.g. "#0A" for 10 (=0xA) outputs
        for (uint8_t output = 0; output < nb_outputs; output++) {
          generate_state_str(output_state_str, output);
          webSocket.sendTXT(WS_client_num, output_state_str);
        }
      }
      break;
    case WStype_TEXT:
      {
        Serial.printf("[%u] get Text: %s\r\n", WS_client_num, payload);

        if (payload[0] == 'p') {  // reply to ping/pong
          webSocket.sendTXT(WS_client_num, "p");
          return;
        }

        if (length != sizeof(output_state_str) -  1) {
          Serial.println("Length mismatch");
          return;
        }

        uint8_t output = hex_to_byte((char*) payload);
        bool state = payload[3] - '0';
        
        if (output >= nb_outputs)  // The request is invalid, because the output doesn't exist
          return;
        Serial.printf("Output %d: %d\r\n", output, state);
        digitalWrite(outputs[output], state);  // Set the selected output accordingly

        webSocket.broadcastTXT(payload);  // broadcast the state change to all connected clients to update their interface
      }
      break;
  }
}

void generate_state_str(char* buff, uint8_t output) {
  buff[0] = nibble_to_hex(output >> 4);
  buff[1] = nibble_to_hex(output);
  // buff[2] = ':';
  bool state = digitalRead(outputs[output]);
  buff[3] = state + '0';
  // buff[4] = '\0';
}

uint8_t hex_to_byte(char* str) {
  return (hex_to_nibble(str[0]) << 4) | hex_to_nibble(str[1]);
}

uint8_t hex_to_nibble(char hex) {
  return hex < 'A' ? hex - '0' : hex - 'A' + 10;
}

