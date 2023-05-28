#include <WiFi.h>
#include "ConnectWifi.h"

ConnectWifi connect;

void setup() {
  connect.begin();

  NTP.begin("ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
  NTP.waitSet();
}

void loop() {
  time_t now = time(nullptr) + 9 * 3600;
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
  delay(1000);
}
