
const char* OTAName = "ESP8266";           // A name and a password for the OTA service
const char* OTAPassword = "espetss";

const char* WiFiHostname = "esp8266";      // Host name on the network
const char* mdnsName = "esp8266";          // Domain name for the mDNS responder

#include "Networkinit.hpp"
#include "OTA.hpp"

/*__________________________________________________________HTTP_DATA__________________________________________________________*/

#define DEBUG

#define HOST "pas-inspiron-3551.lan"
#define URI "/database2.php"
#define PORT 80

//#define THINGSPEAK_WRITE_API_KEY "U03OBDKOR7B0FTJW"

const char* httpHeader =         "POST " URI " HTTP/1.1\r\n"
                                 "Host: " HOST "\r\n"
                                 "Content-Type: application/x-www-form-urlencoded\r\n"
                                 "Connection: close\r\n"
#ifdef THINGSPEAK_WRITE_API_KEY
                                 "THINGSPEAKAPIKEY: " THINGSPEAK_WRITE_API_KEY "\r\n"
#endif
                                 "Content-Length: ";

const char* field_names[] = {"field1", "field2", "field3"};
const size_t nb_fields = sizeof(field_names) / sizeof(field_names[0]);

/*__________________________________________________________SETUP__________________________________________________________*/

void setup() {
  Serial.begin(115200);        // Start the Serial communication to send messages to the computer
  Serial.println("\r\n");

  startWiFi();                 // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection
  startOTA();                  // Start the OTA service
}

/*__________________________________________________________LOOP__________________________________________________________*/

#define SECONDS 1000UL
const unsigned long POST_interval = 20 * SECONDS;
unsigned long last_POST = 0;

void loop() {
  ArduinoOTA.handle();                        // listen for OTA events
  printIP();

  unsigned long now = millis();
  if (now - last_POST > POST_interval) {
    if (WiFi.status() == WL_CONNECTED) {
      int values[nb_fields] = { -5, -3, -1};
      bool success = sendHttpPost(values);
      if (success) {
        Serial.println("POST request successful");
        last_POST = now;
      }
      Serial.println();
    }
  }
}

/*__________________________________________________________SETUP_FUNCTIONS__________________________________________________________*/


/*__________________________________________________________HTTP__________________________________________________________*/

char httpBody[256];  // a buffer for the HTTP body

bool sendHttpPost(int values[nb_fields]) {
  if (nb_fields == 0) {  // if there's no data to be sent, don't send anything, just return
    Serial.println("No data to be sent");
    return false;
  }

  WiFiClient client;  // Create a WiFiClient object
  if (!client.connect(HOST, PORT)) {  // Try to connect to the server
    Serial.println("Connection failed");
    return false;  // return false if it failed
  }

  size_t bodyLen = sprintf(httpBody, "%s=%d", field_names[0], values[0]);
  for (size_t i = 1; i < nb_fields; i++) {
    if (sizeof(httpBody) - bodyLen < 7) // 5 digits + sign + null
      return false;
    bodyLen += sprintf(&httpBody[bodyLen], "&%s=%d", field_names[i], values[i]);
  }

#ifdef DEBUG
  Serial.print(httpHeader);
  Serial.println(bodyLen);
  Serial.println();
  Serial.println(httpBody);
#endif

  client.print(httpHeader);
  client.println(bodyLen);
  client.println();
  client.print(httpBody); // send the request to the server

  client.flush(); // wait for the transmission to finish

  bool HTTPstatusOK = checkHTTPstatusOK(client);

  client.stop(); // Close the connection

#ifdef DEBUG
  Serial.println();
#endif
  return HTTPstatusOK;
}

bool checkHTTPstatusOK(WiFiClient& client) {
  unsigned long timeout = millis();

  /* Read until the first space (SP) of the response. */
  char data = 0;
  while (data != ' ') {
    if (client.available() > 0) {
      data = client.read();
    }
    if (millis() - timeout > 5000) {
      Serial.println("Error: Client Timeout !");
      client.stop();
      return false;
    }
    yield();
  }
  /* The three next characters are the HTTP status code.
      Wait for 3 characters to be received, and read them into a buffer. */
  while (client.available() < 3) {
    if (millis() - timeout > 5000) {
      Serial.println("Error: Client Timeout !");
      client.stop();
      return false;
    }
    yield();
  }
  char statusCodeStr[4];
  client.read((uint8_t*) statusCodeStr, 3);
  statusCodeStr[3] = '\0';

  /* Check if the status code was "200". If so, the request was successful. */
  if (strcmp(statusCodeStr, "200") != 0) {
    Serial.print("Error: HTTP status ");
    Serial.println(statusCodeStr);
    client.stop();
    return false;
  }
  return true;
}

