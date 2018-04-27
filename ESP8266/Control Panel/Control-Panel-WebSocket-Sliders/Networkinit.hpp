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
  if (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) == WL_CONNECT_FAILED)          // Connect to a given access point (specified in WiFi-Credentials.h)
    Serial.println("Connect failed (WL_CONNECT_FAILED)");
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
  static wl_status_t prev = WL_DISCONNECTED;
  wl_status_t status = WiFi.status();
  if (status == prev)
    return;
  prev = status;
  switch(status) {
    case WL_CONNECTED:
      Serial.print("Connected to ");
      Serial.println(WiFi.SSID());
      Serial.print("IP address:\t");
      Serial.println(WiFi.localIP());
      startMDNS();
      break;
    case WL_NO_SSID_AVAIL:
      Serial.println("Error: No SSID available");
      break;
    case WL_CONNECT_FAILED:
      Serial.println("Error: Connect failed");
      break;
    case WL_IDLE_STATUS:
      Serial.println("WiFi idle");
      break;
    case WL_DISCONNECTED:
      Serial.println("WiFi disconnected");
      break;
    default:
      Serial.println("Unknown WiFi status");
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
