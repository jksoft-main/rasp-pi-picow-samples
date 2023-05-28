#include "ConnectWifi.h"

ConnectWifi connect;

void setup() {
  Serial.begin(115200);
  connect.begin();
}

void loop() {
  Serial.printf("WiFi:%s IP:%s\n",
    connect.isConnected() ? "Connected" : "No connect",
    connect.getIp().c_str());
  delay(1000);
}
