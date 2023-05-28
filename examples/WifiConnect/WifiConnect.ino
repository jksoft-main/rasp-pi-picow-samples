#include <WiFi.h>

#define SSID "ssid"
#define PASSWORD "password"

const char* ssid = SSID;
const char* password = PASSWORD;

WiFiMulti multi;

void setup() {
  Serial.printf("Connecting to %s",ssid);
  multi.addAP(ssid, password);
  while (multi.run() != WL_CONNECTED){
    Serial.printf(".");
  }
  Serial.println();
  Serial.println("Wi-Fi connected");
  Serial.print("IP address:");
  Serial.println(WiFi.localIP());
}

void loop() {

}
