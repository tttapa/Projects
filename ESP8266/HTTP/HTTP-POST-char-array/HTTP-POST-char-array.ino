
const char* OTAName = "ESP8266";           // A name and a password for the OTA service
const char* OTAPassword = "espetss";

const char* WiFiHostname = "esp8266";      // Host name on the network
const char* mdnsName = "esp8266";          // Domain name for the mDNS responder

#include "Networkinit.hpp"
#include "OTA.hpp"

/*__________________________________________________________HTTP_DATA__________________________________________________________*/

#define DEBUG  // Print the HTTP requests and other debug information to the serial monitor

/*
   Information about the server 
 */
#define HOST "pas-inspiron-3551.lan"  // The host name of the server (without "http://" and without any slashes)
#define URI "/database2.php"          // The URI of the resource you wish to send the request to
#define PORT 80

/* 
  // Uncomment this to send data to ThingSpeak
  #define HOST "api.thingspeak.com"
  #define URI "/update"
  #define PORT 80

  #define THINGSPEAK_WRITE_API_KEY "U03OBDKOR7B0FTJW"  // The API write key for your ThingSpeak channel
*/

/*
   The HTTP request headers. Everything will be concatenated at compile time, and stored as a static string,
   so no need to dynamically allocate memory or to use a large buffer.
*/
const char* httpHeader =         "POST " URI " HTTP/1.1\r\n"
                                 "Host: " HOST "\r\n"
                                 "Content-Type: application/x-www-form-urlencoded\r\n"
                                 "Connection: close\r\n"
#ifdef THINGSPEAK_WRITE_API_KEY
                                 "THINGSPEAKAPIKEY: " THINGSPEAK_WRITE_API_KEY "\r\n"
#endif
                                 "Content-Length: ";

/* The field or column names in your database. These will be the "keys" of the key-value pairs in the body of the request. */
const char* field_names[] = {"field1", "field2", "field3"};
const size_t nb_fields = sizeof(field_names) / sizeof(field_names[0]);

/*__________________________________________________________SETUP__________________________________________________________*/

void setup() {
  Serial.begin(115200);        // Start the Serial communication to send messages to the computer
  Serial.println("\r\n");

  startWiFi();                 // Change the WiFi hostname, set the WiFi mode,
                               // and connect to the WiFi network specified in WiFi-Credentials.h.
  startOTA();                  // Start the OTA service
}

/*__________________________________________________________LOOP__________________________________________________________*/

const unsigned long POST_interval = 20 * 1000;  // Send a new POST request every 20 seconds
unsigned long last_POST = 0;  // a variable to keep track of the time of the previous request (see Blink Without Delay)

const size_t httpBodySize = 128;  // Make sure it's large enough to fit the entire string you want to send
char httpBody[httpBodySize];  // a buffer for the HTTP body

void loop() {
  ArduinoOTA.handle();                        // listen for OTA events
  printIP();                                  // Check if the WiFi is connected, if so, print the SSID and IP address,
                                              // and start mDNS, else print that it's disconnected

  static int value = 0;                       // Just a test value that will be incremented on each request

  unsigned long now = millis();
  if (now - last_POST > POST_interval) {      // Blink Without Delay
    if (WiFi.status() == WL_CONNECTED) {

      int values[nb_fields] = { value++, -3, -2147483648};         // An array containing the values to send to the server
      size_t bodyLen = generateBodyStr(httpBody, values);          // Generate the string for the body of the request
      bool success = sendHttpPost(httpHeader, httpBody, bodyLen);  // Send the HTTP request

      if (success) {
        Serial.println("HTTP request successful");
      }
      Serial.println();
      last_POST += POST_interval;
    }
  }
}

/*__________________________________________________________HTTP__________________________________________________________*/

/*
    Generate the body url-encoded string, based on the field names and the respective values.
    Write this string to the httpBody buffer.
    Returns the number of characters it wrote.
*/
size_t generateBodyStr(char* httpBody, int values[nb_fields]) {
  if (nb_fields == 0)
    return 0;
  // print the first field name and value as key=value pair
  size_t bodyLen = sprintf(httpBody, "%s=%d", field_names[0], values[0]);  
  for (size_t i = 1; i < nb_fields; i++) {  // append the remaining field names and values to the string
    // each pair is separated by an ampersand (&)
    bodyLen += sprintf(&httpBody[bodyLen], "&%s=%d", field_names[i], values[i]);
  }
  return bodyLen;
}

/*
   Send the HTTP POST request to the server.
   Takes char arrays of the headers and body, plus the length of the body as parameters.
*/
bool sendHttpPost(const char* httpHeader, char* httpBody, size_t bodyLen) {
  if (bodyLen == 0) {  // if there's no data to be sent, don't send anything, just return false
    Serial.println("\r\n\nNo data to be sent");
    return false;
  }

#ifdef DEBUG
  Serial.println("\r\nSending HTTP request");
  Serial.print("\tTime: ");
  Serial.println(millis());
  Serial.println();
  Serial.print(httpHeader);
  Serial.println(bodyLen);
  Serial.println();
  Serial.println(httpBody);
#endif

  /* Connect to the server */
  WiFiClient client;  // Create a WiFiClient object
  if (!client.connect(HOST, PORT)) {  // Try to connect to the server
    Serial.println("\r\nError:\tConnection failed");
    return false;  // return false if it failed
  }
  
  /* Send the request to the server, 
     write headers, lenght of the body and body over the TCP connection */
  client.print(httpHeader);
  client.println(bodyLen);
  client.println();
  client.print(httpBody);

  client.flush(); // wait for the transmission to finish

  /* Check the server's response to see if the request was successful */
  bool HTTPstatusOK = checkHTTPstatusOK(client);

  client.stop(); // Close the connection

#ifdef DEBUG
  Serial.println();
#endif

  return HTTPstatusOK;
}

/*
   Check the server's response and check the HTTP status code to see if the request was successful.
   Returns true on success.
*/
bool checkHTTPstatusOK(WiFiClient& client) {
  unsigned long timeout = millis();

  /* Read until the first space (SP) of the response. */
  char data = 0;
  while (data != ' ') {  // wait for a space character
    if (client.available() > 0) {  // if theres a character to read from the response
      data = client.read();  // read it
    }
    // if the space doesn't arrive within 5 seconds, timeout and return
    if (millis() - timeout > 5000) {  
      Serial.println("Error: Client Timeout !");
      return false;
    }
    yield();
  }
  
  /* The three next characters are the HTTP status code.
      Wait for 3 characters to be received, and read them into a buffer. */
  while (client.available() < 3) {
    if (millis() - timeout > 5000) {
      Serial.println("\r\nError:\tClient Timeout !");
      return false;
    }
    yield();
  }
  char statusCodeStr[4];  // three digits + null
  client.read((uint8_t*) statusCodeStr, 3);  // read the status code into the buffer
  statusCodeStr[3] = '\0';  // add terminating null character, needed for strcmp()

  /* Check if the status code was "200". If so, the request was successful. */
  if (strcmp(statusCodeStr, "200") != 0) {
    Serial.print("\r\nError:\tHTTP status ");
    Serial.println(statusCodeStr);
    return false;
  }
  
  return true;
}

