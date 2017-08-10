#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include "WiFi-Credentials.h"

/*
   Change the WiFi hostname, set the WiFi mode,
   and connect to the WiFi network specified in WiFi-Credentials.h.
*/
void startWiFi() {
  WiFi.hostname(WiFiHostname);
  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);          // Connect to a given access point (specified in WiFi-Credentials.h)
}

/*
   Start the mDNS responder.
*/
void startMDNS() {
  static bool mDNSstarted = false;
  if (mDNSstarted)
    return;
  if (!MDNS.begin(mdnsName)) {                       // start the multicast domain name server
    Serial.println("Couldn't start mDNS responder");
    return;
  }
  Serial.print("mDNS responder started: http://");
  Serial.print(mdnsName);
  Serial.println(".local");
  mDNSstarted = true;
}

/*
   Check if the WiFi is connected, if so, print the SSID and IP address, and start mDNS,
   else print that it's disconnected.
*/
void printIP() {
  static boolean printed = false;
  if (WiFi.status() == WL_CONNECTED) {
    if (printed)
      return;
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());
    Serial.print("\tTime: ");
    Serial.println(millis());
    startMDNS();
    Serial.println();
    printed = true;
  } else {  // if WiFi is not connected
    if (!printed)
      return;
    Serial.println("WiFi disconnected!");
    printed = false;
  }
}
