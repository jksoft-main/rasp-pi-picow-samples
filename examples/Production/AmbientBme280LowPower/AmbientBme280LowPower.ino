#include <Wire.h>
#include "Seeed_BME280.h"
#include "AmbientRspPico.h"
#include "ConnectWifi.h"
// set_sys_clock_khz関数を使用するため
#include "pico/stdlib.h"
// vreg_set_voltage関数を使用するため
#include "hardware/vreg.h"

// AmbientのチャネルIDを設定する
#define AMBIENT_CHANNEL_ID  0
// Ambientのライトキーを設定する
#define AMBIENT_WRITE_KEY   "write key"
// 計測周期
#define INTERVAL_TIME       (10 * 60 * 1000)

BME280 bme280;
ConnectWifi connect;
Ambient ambient;

unsigned int channelId = AMBIENT_CHANNEL_ID;
const char* writeKey = AMBIENT_WRITE_KEY;
unsigned long intervalTime = INTERVAL_TIME;
float powerVoltage = 0.0;

void setup() {
  // 電源電圧を計測
  pinMode(25,OUTPUT);
  digitalWrite(25,HIGH);
  powerVoltage = analogRead(A3)* 3.3 / 1023.0;
  powerVoltage *= 3;

  // BME280の初期設定
  Wire.setSDA(8);
  Wire.setSCL(9);
  if(!bme280.init()){
    // BME280の設定に失敗したら再起動する
    Serial.println("BME280 initialization error!");
    delay(10000);
    rp2040.reboot();
  }
  // Wi-Fiに接続
  connect.begin();
  // Ambientに接続するための初期設定
  ambient.begin(channelId, writeKey);
}

void loop() {
  unsigned long startTime = millis();
  // Wi-Fiに接続されているかチェックする
  if(connect.isConnected()){
    // BME280から環境情報を取得してAmbientに送信
    ambient.set(1,bme280.getTemperature());
    ambient.set(2,(int)bme280.getHumidity());
    ambient.set(3,bme280.getPressure()/100.0);
    ambient.set(4,powerVoltage);
    ambient.send();
  }

  // USBから供給されているか電池かを判断する
  if(digitalRead(34)){
    // USBから供給されている場合はウェイトのみ
    delay(intervalTime - (millis()-startTime));
  } else {
    // 電池の場合は、省電力化してウェイトしてから再起動
    // BME280をスリープ
    delay(10);
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(BME280_REG_CONTROL);
    Wire.write(0x00);
    Wire.endTransmission();
    delay(10);
    // I2Cを終了
    Wire.end();
    // APとの接続を終了し、Wi-Fiをスリープ
    WiFi.disconnect(true);
    // CPUクロックを20MHzにダウン
    set_sys_clock_khz(20000, true);
    // Vregの電圧を下げる
    vreg_set_voltage(VREG_VOLTAGE_0_95);
    // 起動してからの時間も計算してウェイト
    delay(intervalTime - millis());
    // 再起動
    rp2040.reboot();
  }
}
