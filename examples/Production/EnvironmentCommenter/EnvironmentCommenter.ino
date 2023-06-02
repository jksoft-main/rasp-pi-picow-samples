#include "ConnectWifi.h"
#include <HTTPClient.h>
#include <LovyanGFX.hpp>
#include <Wire.h>
#include "Seeed_BME280.h"
#include <ArduinoJson.h>

// ChatGPTのsecret keyを設定する
#define CHATGPT_SECRET_KEY  "chargpt secret key"

// OLEDディスプレイのSH1107を使うための定義用のクラス
class LGFX_SH1107 : public lgfx::LGFX_Device {
  lgfx::Panel_SH110x   _panel_instance;
  lgfx::Bus_I2C   _bus_instance;

  public:
    LGFX_SH1107() {
      {
        auto cfg = _bus_instance.config();
        cfg.i2c_port    = 1;      // I2Cのポート番号(Wire1)
        cfg.freq_write  = 400000; 
        cfg.freq_read   = 400000;
        cfg.pin_sda     = 6;      // GroveシールドのI2C1が繋がっているピン番号
        cfg.pin_scl     = 7;      // GroveシールドのI2C1が繋がっているピン番号
        cfg.i2c_addr    = 0x3C;   // OLEDディスプレイのI2Cアドレス

        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
      }
      {
        auto cfg = _panel_instance.config();
        // OLEDディスプレイを横向きに使うための設定
        cfg.memory_width  = 64;
        cfg.memory_height = 128;
        cfg.offset_x      = 32;
        cfg.offset_y      = 0;
        cfg.offset_rotation = 3;

        _panel_instance.config(cfg);
      }
      setPanel(&_panel_instance);
    }
};

BME280 bme280;
ConnectWifi connect;
static LGFX_SH1107 lcd;
static LGFX_Sprite canvas(&lcd);
const char* chatGptSecretkey = "Bearer " CHATGPT_SECRET_KEY;

String requestChatGPT(
  const String message
){
  String responseMessage ="取得できませんでした";

  if(connect.isConnected()){
    HTTPClient https;
    https.setInsecure();
    if (https.begin("https://api.openai.com/v1/chat/completions")) {
      https.addHeader("Content-Type", "application/json; charset=utf-8");
      https.addHeader("Authorization", chatGptSecretkey);
      String body = 
        "{\"model\":\"gpt-3.5-turbo\",\"messages\":[{\"role\":\"user\",\"content\":\""
        + message
        + "\"}]}";
      int status = https.POST(body.c_str());
      if (status > 0) {
        String data = https.getString();
        
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, data);
        if (!error) {
          JsonObject choices_0 = doc["choices"][0];
          responseMessage = String(choices_0["message"]["content"]);
        }
      }
      https.end();
    }
  }

  return responseMessage;
}

String getEnvironmentalAssessmentForChatGPT(
  float temperature,float humidity,float pressure
){
  String message = "気温が"
    + String(temperature)
    + "度、湿度が"
    + String(humidity)
    + "%、気圧が"
    + String(pressure)
    + "hPaです。どんな風にすごしたら良いかを30文字以内で教えて";

  return requestChatGPT(message);
}

void display(const String dispayData,const lgfx::U8g2font* font){
  canvas.fillScreen(TFT_BLACK);
  canvas.setCursor(0, 0);
  canvas.setFont(font);
  canvas.println(dispayData);
  canvas.pushSprite(0, 0);
}

void setup() {
  // BME280の初期設定
  Wire.setSDA(8);
  Wire.setSCL(9);
  if(!bme280.init()){
    // BME280の設定に失敗したら再起動する
    Serial.println("BME280 initialization error!");
    delay(10000);
    rp2040.reboot();
  }
  // ディスプレイの初期設定
  lcd.init();
  // 画面右で折り返す設定
  canvas.setTextWrap(true);
  // 画面サイズでキャンバス用のメモリを確保
  canvas.createSprite(lcd.width(), lcd.height());
  // 黒で塗りつぶし
  canvas.fillScreen(TFT_BLACK);
  // 文字色は白
  canvas.setTextColor(TFT_WHITE);

  // Wi-Fiに接続
  connect.begin();
}

void loop() {
  // BME280から環境情報を取得する
  float temperature = bme280.getTemperature();
  int humidity = (int)bme280.getHumidity();
  float pressure = bme280.getPressure()/100.0;

  if(BOOTSEL){
    display("問い合わせ中",&fonts::lgfxJapanMincho_20);
    display(getEnvironmentalAssessmentForChatGPT(
      temperature,
      (float)humidity,
      pressure
      ),
      &fonts::lgfxJapanMincho_16
    );
    delay(10000);
  } else {
    String displayData = "温度:" + String(temperature,1) + "℃\n";
    displayData += "湿度:" + String(humidity) + "％\n";
    displayData += "気圧:" + String(pressure,0) + "hPa\n";
    display(displayData,&fonts::lgfxJapanMincho_20);
  }

  delay(100);
}
