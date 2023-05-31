#include "ConnectWifi.h"
#include <HTTPClient.h>
#include <LovyanGFX.hpp>
#include <Wire.h>
#include "Seeed_BME280.h"
#include <ArduinoJson.h>

#define CHATGPT_TOKEN "chat gpt token"

const char* chatGptToken = "Bearer " CHATGPT_TOKEN;

class LGFX_SH1107 : public lgfx::LGFX_Device {
  lgfx::Panel_SH110x   _panel_instance;
  lgfx::Bus_I2C   _bus_instance;

  public:
    LGFX_SH1107() {
      {
        auto cfg = _bus_instance.config();
        cfg.i2c_port    = 1;
        cfg.freq_write  = 400000;
        cfg.freq_read   = 400000;
        cfg.pin_sda     = 6;
        cfg.pin_scl     = 7;
        cfg.i2c_addr    = 0x3C;

        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
      }
      {
        auto cfg = _panel_instance.config();
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

char* getEnvironmentalAssessmentForChatGPT(
  float temperature,float humidity,float pressure
){
  if(connect.isConnected()){
    HTTPClient https;
    https.setInsecure();
    if (https.begin("https://api.openai.com/v1/chat/completions")) {
      https.addHeader("Content-Type", "application/json; charset=utf-8");
      https.addHeader("Authorization", chatGptToken);
      String message = "気温が" + String((int)temperature)
        + "度、湿度が"
        + String((int)humidity)
        + "%、気圧が"
        + String((int)pressure)
        + "hPaです。どんな風にすごしたら良いかを40文字で教えて";
      String body = 
        "{\"model\":\"gpt-3.5-turbo\",\"messages\":[{\"role\":\"user\",\"content\":\""
        + message
        + "\"}]}";
      Serial.println("req:");
      Serial.println(body);
      int status = https.POST(body.c_str());
      String data = https.getString();
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, data);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return NULL;
      }
      JsonObject choices_0 = doc["choices"][0];
      const char* choices_0_message_content = choices_0["message"]["content"];
      Serial.println("status:");
      Serial.println(status);
      Serial.println("res:");
      Serial.println(data);
      Serial.println("message:");
      Serial.println(choices_0_message_content);
      https.end();

      return (char*)choices_0_message_content;
    }
  }

  return NULL;
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
  canvas.setTextWrap(true);
  canvas.createSprite(lcd.width(), lcd.height());
  canvas.fillScreen(TFT_BLACK);
  canvas.setTextColor(TFT_WHITE);

  // Wi-Fiに接続
  connect.begin();
}

void loop() {
  static bool displayMode = false;

  // BME280から環境情報を取得する
  float temperature = bme280.getTemperature();
  int humidity = (int)bme280.getHumidity();
  float pressure = bme280.getPressure()/100.0;

  if(displayMode){
    // Wi-Fiに接続されているかチェックする
    if(connect.isConnected()){
      canvas.fillScreen(TFT_BLACK);
      canvas.setCursor(0, 0);
      canvas.setFont(&fonts::lgfxJapanMincho_12);
      canvas.println(getEnvironmentalAssessmentForChatGPT(temperature,(float)humidity,pressure));
      canvas.pushSprite(0, 0);
      delay(10000);
      displayMode = false;
    }
  } else {
    canvas.fillScreen(TFT_BLACK);
    canvas.setCursor(0, 0);
    canvas.setFont(&fonts::lgfxJapanMincho_20);
    canvas.printf("温度:%.1f℃\n湿度:%d％\n気圧:%dhPa\n",temperature,humidity,(int)pressure);
    canvas.pushSprite(0, 0);
  }

  if(BOOTSEL){
    displayMode = !displayMode;
  }

  delay(100);
}
