#include "HX711.h"

const int HX711_DOUT_PIN = 19;
const int HX711_SCK_PIN = 18;
const int PIR_SENSOR_PIN = 20;

HX711 scale;

long baseScale = 0;
bool OnTheKey,nowOnTheKey = false;

void setup() {
  Serial.begin(115200);
  pinMode(PIR_SENSOR_PIN, INPUT);
  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  if (scale.is_ready()) {
    baseScale = scale.read();
    Serial.printf("HX711 reading base: %lu\n",baseScale);
  }
}

void loop() {
  if (scale.is_ready()) {
    long reading = scale.read();
    long difference = abs(reading - baseScale);
    int pirSensor = digitalRead(PIR_SENSOR_PIN);
    static int beforPirSensor = pirSensor;
    if(constrain(difference, -2000,2000) == difference){
      OnTheKey = false;
    } else {
      OnTheKey = true;
    }
    if(beforPirSensor == LOW && pirSensor == HIGH){
      nowOnTheKey = OnTheKey;
    }
    if(beforPirSensor == HIGH && pirSensor == LOW){
      if(nowOnTheKey){
        if(OnTheKey){
          Serial.printf("何もなし\n");
        } else {
          Serial.printf("外出します\n");
        }
      } else {
        if(OnTheKey){
          Serial.printf("帰ってきました\n");
        } else {
          Serial.printf("鍵を置き忘れてませんか\n");
        }
      }
    }
    beforPirSensor = pirSensor;
    //Serial.printf("scale: %lu \tbaseScale: %d\n",reading,baseScale);
    Serial.printf("difference: %lu \tOnTheKey: %d \tSensor: %d\n",difference,OnTheKey,pirSensor);
  } else {
    Serial.println("HX711 not found.");
  }

  delay(1000);
  
}
