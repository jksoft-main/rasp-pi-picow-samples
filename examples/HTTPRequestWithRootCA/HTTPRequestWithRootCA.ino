#include "ConnectWifi.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define API_KEY "api key"
#define AREA    "Tokyo, JP"

ConnectWifi connect;

const char* rootCa =  R"EOF(
  ここにルート証明書を貼り付ける
)EOF";

void setup() {
  connect.begin();

  String url = "https://api.openweathermap.org";
  url += "/data/2.5/forecast";
  url += "?q=" AREA;
  url += "&units=metric";
  url += "&lang=ja";
  url += "&cnt=2";
  url += "&appid=" API_KEY;

  HTTPClient https;
  https.setCACert(rootCa);
  if(https.begin(url)){
    int status = https.GET();
    if (status > 0) {
      Serial.printf("status:%u",status);
      String data = https.getString();

      DynamicJsonDocument doc(2048);
      deserializeJson(doc, data);
      JsonObject list_1 = doc["list"][1];
      JsonObject list_1_main = list_1["main"];
      JsonObject weather = list_1["weather"][0];
      float temp = list_1_main["temp"];
      float hum = list_1_main["humidity"];
      float pressure = list_1_main["pressure"];
      const char* description = weather["description"];

      Serial.printf("気温:%.2f\n",temp);
      Serial.printf("湿度:%.2f\n",hum);
      Serial.printf("気圧:%.2f\n",pressure);
      Serial.printf("天気:%s\n",description);
    } else {
      Serial.printf("status:%u\n",status);
    }
    https.end();
  } else {
    Serial.printf("error\n");
  }
}

void loop() {}
