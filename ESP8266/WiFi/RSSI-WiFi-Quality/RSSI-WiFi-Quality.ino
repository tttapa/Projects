#include <ESP8266WiFi.h>

#include "WiFi-Credentials.h"

void setup() {
  Serial.begin(115200);
  Serial.println("\r\n");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  // Try to connect to a given WiFi network
}

void loop() {
  printWiFiState();

  static int previousQuality = -1;
  int quality = getQuality();
  if (quality != previousQuality) {  // If the quality changed since last print, print new quality and RSSI
    if (quality != -1)
      Serial.printf("WiFi Quality:\t%d\%\tRSSI:\t%d dBm\r\n", quality, WiFi.RSSI());
    previousQuality = quality;
  }
}

/*
   If WiFi is connected, print the SSID and the IP address.
   If the WiFi is disconnected, print that it's disconnected.
*/
void printWiFiState() {
  static boolean printed = false;
  if (WiFi.status() == WL_CONNECTED) {
    if (printed)
      return;
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());
    Serial.println();
    printed = true;
  } else {  // if WiFi is not connected
    if (!printed)
      return;
    Serial.println("WiFi disconnected!\n");
    printed = false;
  }
}

/*
   Return the quality (Received Signal Strength Indicator)
   of the WiFi network.
   Returns a number between 0 and 100 if WiFi is connected.
   Returns -1 if WiFi is disconnected.
*/
int getQuality() {
  if (WiFi.status() != WL_CONNECTED)
    return -1;
  int dBm = WiFi.RSSI();
  if (dBm <= -100)
    return 0;
  if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}
