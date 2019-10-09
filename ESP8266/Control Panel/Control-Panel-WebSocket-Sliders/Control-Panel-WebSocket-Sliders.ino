#define STATION  // Try to connect as a station as well as starting a software Access Point

const char* AP_ssid = "ESP8266 Access Point";  // The name of the Wi-Fi network that will be created
const char* AP_password = "thereisnospoon";    // The password required to connect to it, leave blank for an open network

const char* OTAName = "ESP8266";           // A name and a password for the OTA service
const char* OTAPassword = "espetss";

const char* WiFiHostname = OTAName;        // Host name on the network

#include "Networkinit.hpp"
#include "OTA.hpp"
#include "Server.hpp"

#include <WebSocketsServer.h>

WebSocketsServer webSocket(81);


const uint8_t nb_outputs = 1;            // the total number of outputs
uint8_t output_values[nb_outputs] = {};  // an array to keep track of the value of each output

char nb_outputs_str[4] = "#AA";  // "#AA" + null, string message to let the client know how many (AA) sliders to show
char output_value_str[6] = "FF:VV";  // "FF:VV" + null, string message to let the client know what the value (VV) of an output (FF) is

unsigned long lastClientActivity[WEBSOCKETS_SERVER_CLIENT_MAX];  // an array with timestamps of the last messages from all clients
bool connectedClients[WEBSOCKETS_SERVER_CLIENT_MAX] = {};  // an array that keeps track of which clients are connected
const unsigned long clientTimeout = 10000;  // disconnect clients that don't send any messages for more than 10 seconds

/*__________________________________________________________SETUP__________________________________________________________*/

void setup() {

  Serial.begin(115200);        // Start the Serial communication to send messages to the computer
  Serial.println("\r\n");

  startWiFi();                 // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection

  startOTA();                  // Start the OTA service

  startSPIFFS();               // Start the SPIFFS and list all contents

  startServer();               // Start a HTTP server with a file read handler and an upload handler

  startMDNS();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  generate_nb_outputs_str();

  pinMode(LED_BUILTIN, OUTPUT);
  analogWrite(LED_BUILTIN, 1023);

}

/*__________________________________________________________LOOP__________________________________________________________*/

void loop() {
  server.handleClient();                      // run the server
  ArduinoOTA.handle();                        // listen for OTA events
  webSocket.loop();                           // run the WebSocket server
  printIP();
  printStations();
}

/*__________________________________________________________MESSAGE_STRING_FUNCTIONS__________________________________________________________*/

void generate_nb_outputs_str() {  // convert the number of outputs to a string message
  // nb_outputs_str[0] = '#';
  byte_to_str(&nb_outputs_str[1], nb_outputs);
}

void byte_to_str(char* buff, uint8_t val) {  // convert an 8-bit byte to a string of 2 hexadecimal characters
  buff[0] = nibble_to_hex(val >> 4);
  buff[1] = nibble_to_hex(val);
  // buff[2] = '\0';
}

char nibble_to_hex(uint8_t nibble) {  // convert a 4-bit nibble to a hexadecimal character
  nibble &= 0xF;
  return nibble > 9 ? nibble - 10 + 'A' : nibble + '0';
}

void generate_value_str(char* buff, uint8_t output) {  // convert the index of an output and its value to a string message
  byte_to_str(buff, output);
  // buff[2] = ':';
  uint8_t value = output_values[output];
  byte_to_str(&buff[3], value);
  // buff[5] = '\0';
}

uint8_t hex_to_byte(char* str) {  // parse a string of 2 hexadecimal characters to an 8-bit byte
  return (hex_to_nibble(str[0]) << 4) | hex_to_nibble(str[1]);
}

uint8_t hex_to_nibble(char hex) {  // convert a hexadecimal character to a 4-bit nibble
  return hex < 'A' ? hex - '0' : hex - 'A' + 10;
}

bool validOutputChangeMsg(uint8_t* payload, size_t length) {  // check if a string has the format of a message to change the output ("FF:VV")
  return (length == sizeof(output_value_str) -  1)     // the length should be 5 (two chars for the output index, one colon, two characters for the value)
         && isHexChar(payload[0])                      // the output index consists of 2 hexadecimal characters
         && isHexChar(payload[1])
         && (payload[2] == ':')                        // output index and value are separated by a colon
         && isHexChar(payload[3])                      // the value consists of 2 hexadecimal characters
         && isHexChar(payload[4]);
}

bool isHexChar(char hex) {  // check if a given character is a hexadecimal number (0-9 or A-F)
  return (hex >= '0' && hex <= '9') || (hex >= 'A' && hex <= 'F');
}

/*__________________________________________________________WEBSOCKET__________________________________________________________*/

void webSocketEvent(uint8_t WS_client_num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", WS_client_num);
      connectedClients[WS_client_num] = false;
      break;
    case WStype_CONNECTED:  // When a new client connects
      {
        IPAddress ip = webSocket.remoteIP(WS_client_num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", WS_client_num, ip[0], ip[1], ip[2], ip[3], payload);

        connectedClients[WS_client_num] = true;  // add it to the list of connected clients
        lastClientActivity[WS_client_num] = millis();  // keep the "last activity" time up to date
        sendTXTDebug(WS_client_num, nb_outputs_str);  // send the number of outputs, e.g. "#0A" for 10 (=0xA) outputs
        // when the client receives this, it will add the right number of sliders to the web page
        sendAllStates(WS_client_num);  // send all output values to the new client, the client will set the new sliders to the right position
      }
      break;
    case WStype_TEXT:  // When a client sends a text message
      {
        Serial.printf("[%u] get Text: %s\r\n", WS_client_num, payload);
        lastClientActivity[WS_client_num] = millis();  // keep the "last activity" time up to date

        if (payload[0] == 'p') {  // reply to ping request
          sendTXTDebug(WS_client_num, "p");
          return;
        }

        if (payload[0] == '?') {  // send all output values to the client, the client will update all sliders
          // the client requests this when it has been offline for some time
          sendAllStates(WS_client_num);
          return;
        }

        if (!validOutputChangeMsg(payload, length)) {  // if the message is not "p", "?" or a valid output change message, ignore message and return
          Serial.println("Invalid message");
          return;
        }

        uint8_t output = hex_to_byte((char*) payload);      // parse the output change message
        uint8_t value  = hex_to_byte((char*) &payload[3]);

        if (output >= nb_outputs) {  // if the output index doesn't exist, the request is invalid
          Serial.println("Invalid output index");
          return;
        }
        output_values[output] = value;
        setOutput(output, value);  // Set the selected output accordingly

        broadcastDebug(payload);  // broadcast the new value to all connected clients to update their interface
      }
      break;
  }
}

void sendAllStates(uint8_t WS_client_num) {  // send the values of all outputs to a given client
  for (uint8_t output = 0; output < nb_outputs; output++) {
    generate_value_str(output_value_str, output);
    sendTXTDebug(WS_client_num, output_value_str);
  }
}

void broadcastDebug(uint8_t* payload) {  // broadcast a text message over the WebSocket connnection, and print the contents and the duration of the transmission
  disconnectOldClients();

  Serial.print("\tBroadcasting: ");
  Serial.print((char*) payload);
  Serial.print("\t(");
  unsigned long start = millis();
  webSocket.broadcastTXT(payload);
  Serial.print(millis() - start);
  Serial.println(')');
}

void sendTXTDebug(uint8_t num, char* payload) {  // send a text message to a given WebSocket client, and print the contents and the duration of the transmission
  disconnectOldClients();

  Serial.printf("\tSending [%d]: ", num);
  Serial.print(payload);
  Serial.print("\t(");
  unsigned long start = millis();
  webSocket.sendTXT(num, payload);
  Serial.print(millis() - start);
  Serial.println(')');
}

void disconnectOldClients() {  // if clients haven't sent anything for more than 10 seconds, disconnect them
  for (uint8_t num = 0; num < WEBSOCKETS_SERVER_CLIENT_MAX; num++) {
    if (millis() - lastClientActivity[num] > clientTimeout && connectedClients[num]) {
      Serial.printf("[%u] Timeout\r\n", num);
      webSocket.disconnect(num);
      connectedClients[num] = false;
    }
  }
}

void setOutput(uint8_t output, uint8_t value) {
  Serial.printf("Output %d: %d\r\n", output, value);
  if (output == 0)
    analogWrite(LED_BUILTIN, 1020 - 4 * value);
}
