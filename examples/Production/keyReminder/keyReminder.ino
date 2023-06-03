#include "ConnectWifi.h"
#include <HTTPClient.h>
#include "HX711.h"

// LINEアクセストークン
#define LINE_ACCESS_TOKEN "LINE access token"

const char* lineAccessToken = "Bearer " LINE_ACCESS_TOKEN;

// ロードセル用変換回路が接続されているピン
const int HX711_DOUT_PIN = 19;
const int HX711_SCK_PIN = 18;
// 人感センサーが接続されているピン
const int PIR_SENSOR_PIN = 20;

ConnectWifi connect;
HX711 scale;

long baseScale = 0;

void lineNotify(
  const String message
){
  if(connect.isConnected()){
    HTTPClient https;
    https.setInsecure();
    if (https.begin("https://notify-api.line.me/api/notify")) {
      https.addHeader("Content-Type", "application/x-www-form-urlencoded");
      https.addHeader("Authorization", lineAccessToken);
      String body = "message=" + message;
      int status = https.POST(body.c_str());
      if (status == 200){
        Serial.println("LINE通知完了");
      } else {
        Serial.printf("LINE通知エラー ステータス:%d", status);
      }
      https.end();
    }
  }
}


void setup() {
  delay(1000);
  
  // 焦電センサー用のピンを入力に設定
  pinMode(PIR_SENSOR_PIN, INPUT);
  // 重量センサーの初期設定
  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  if (scale.is_ready()) {
    // 重量の基準を保持
    baseScale = scale.read();
  } else {
    // 重量センサーの設定に失敗したら再起動する
    Serial.println("HX711 initialization error!");
    delay(10000);
    rp2040.reboot();
  }
  // Wi-Fiに接続
  connect.begin();
}

void loop() {
  if (scale.is_ready()) {
    bool keyStatus = false;
    static bool pirSensingKeyStatus = false;
    // 重量センサー値を読み込み
    long reading = scale.read();
    // 基準との差分を計算
    long difference = abs(reading - baseScale);
    // 人感センサーの状態を読み込み
    int pirSensor = digitalRead(PIR_SENSOR_PIN);
    // 人感センサーの前回値用
    static int beforPirSensor = pirSensor;
    // 人感センサーの状態でLEDを点灯
    digitalWrite(LED_BUILTIN,!pirSensor);
    // 鍵が置いてあるかどうかを判定
    if(constrain(difference, -2000,2000) == difference){
      keyStatus = false;
    } else {
      keyStatus = true;
    }
    // 人感センサーが反応したタイミングの鍵の状態を保持
    if(beforPirSensor == LOW && pirSensor == HIGH){
      pirSensingKeyStatus = keyStatus;
    }
    // 人感センサーの反応が無くなったタイミングで値を評価
    if(beforPirSensor == HIGH && pirSensor == LOW){
      if(pirSensingKeyStatus){
        // 人感センサーが反応したときに鍵がある場合
        if(!keyStatus){
          // 鍵がなくなった
          lineNotify("いってらっしゃい！");
        }
      } else {
        // 人感センサーが反応したときに鍵が無い場合
        if(keyStatus){
          // 鍵が置かれた
          lineNotify("お帰りなさい！");
        } else {
          // 鍵が置かれてない
          lineNotify("鍵を置き忘れてない！？");
        }
      }
    }
    // 人感センサーの前回値を保持
    beforPirSensor = pirSensor;
  }

  delay(1000);
}
