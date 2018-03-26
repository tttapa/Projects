#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#ifdef STATION
#include "WiFi-Credentials.h"
#endif

void startWiFi() {  // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection
  WiFi.hostname(WiFiHostname);
#ifdef STATION
  WiFi.mode(WIFI_AP_STA);
#else
  WiFi.mode(WIFI_AP);
#endif
  WiFi.disconnect();
  WiFi.softAP(AP_ssid, AP_password);             // Start the access point
  Serial.print("Access Point \"");
  Serial.print(AP_ssid);
  Serial.println("\" started\r\n");
#ifdef STATION
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);          // Connect to a given access point (specified in WiFi-Credentials.h)
#endif
}

void startMDNS() {  // Start the mDNS responder
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

void printIP() {
#ifdef STATION
  static boolean printed = false;
  if (WiFi.status() == WL_CONNECTED) {
    if (printed)
      return;
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());
    startMDNS();
    printed = true;
  } else {
    printed = false;
  }
#endif
}

void printStations() {
  static int prevNumber = 0;
  if (WiFi.softAPgetStationNum() != prevNumber) {
    if (WiFi.softAPgetStationNum() > 0) {
      startMDNS();
    }
    prevNumber = WiFi.softAPgetStationNum();
    Serial.print(prevNumber);
    Serial.println(" station(s) connected");
  }
}
