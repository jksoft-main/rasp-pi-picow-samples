#include "ConnectWifi.h"

ConnectWifi::ConnectWifi(const char* wifi_setting_ip,const char* wifi_setting_subnet,const int wifi_setting_port,const int status_pin)
  : _wifi_setting(true),_status_pin(status_pin),_wifi_setting_port(80){
  _wifi_setting_ip = new IPAddress();
  _wifi_setting_ip->fromString(wifi_setting_ip);
  _wifi_setting_subnet = new IPAddress();
  _wifi_setting_subnet->fromString(wifi_setting_subnet);
}

ConnectWifi::ConnectWifi(const bool wifi_setting,const char* ssid,const char* password,const int status_pin)
  : _wifi_setting(wifi_setting),_status_pin(status_pin),_wifi_setting_port(80){
  if(!_wifi_setting){
    _ssid = new String(ssid);
    _password = new String(password);
  } else {
    _wifi_setting_ip = new IPAddress((const uint8_t*)DEFAULT_WIFI_SETTING_IP);
    _wifi_setting_subnet = new IPAddress((const uint8_t*)DEFAULT_WIFI_SETTING_SUBNET);
  }
}

bool ConnectWifi::isValidSSID(const char* ssid) {
  int i, len;
  len = strlen(ssid);
  
  if(len < 1 || len > 32) {
    return false;
  }
  
  for(i = 0; i < len; i++) {
    char c = ssid[i];
    if(c < '0' || c >'9'){
      if(c < 'a' || c > 'z'){
        if(c < 'A' || c > 'Z'){
          if( c != '_' && c != '-'){
            return false;
          }
        }
      }
    }
  }
  
  return true;
}

bool ConnectWifi::isValidPassword(const char* password) {
  int i, len;
  len = strlen(password);
  
  if(len < 8 || len > 63) {
    return false;
  }
  
  for(i = 0; i < len; i++) {
    char c = password[i];
    if(c < '0' || c >'9'){
      if(c < 'a' || c > 'z'){
        if(c < 'A' || c > 'Z'){
          return false;
        }
      }
    }
  }
  
  return true;
}

bool ConnectWifi::saveWiFiSettings(const char* ssid,const char* passoword){
  int ssid_size = strlen(ssid);
  int password_size = strlen(passoword);
  int write_pointer = 0;

  if(ssid_size+password_size+2 > EEPROM.length()){
    return false;
  }

  if(!isValidSSID(ssid)){
    return false;
  }

  if(!isValidPassword(passoword)){
    return false;
  }

  for(int i=0 ; i<ssid_size ; i++,write_pointer++){
    EEPROM.write(write_pointer, ssid[i]);
  }
  EEPROM.write(write_pointer++, 0x00);

  for(int i=0 ; i<password_size ; i++,write_pointer++){
    EEPROM.write(write_pointer, passoword[i]);
  }
  EEPROM.write(write_pointer++, 0x00);

  return EEPROM.commit();
}

void ConnectWifi::captivePortal() {
  connectWifiServer->sendHeader("Location", String("http://") + connectWifiServer->client().localIP().toString(), true);
  connectWifiServer->send(302, "text/plain", "");
  connectWifiServer->client().stop();
}

void ConnectWifi::handleRoot() {
  auto cnt = WiFi.scanNetworks();

  String html = "<!DOCTYPE html><html lang='ja'><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Wi-Fiのアクセスポイント設定</title>";
  html += "<link href='https://cdnjs.cloudflare.com/ajax/libs/materialize/1.0.0/css/materialize.min.css' rel='stylesheet'>";
  html += "<style>.wifi-card{margin:20px 0;padding:20px;border-radius:10px;background-color:#f9f9f9;box-shadow:0px 4px 8px rgba(0, 0, 0, 0.1);}";
  html += ".wifi-card:hover{background-color:#e0f7fa;transition:0.3s;}.wifi-header{display:flex;justify-content:space-between;align-items:center;}";
  html += ".wifi-header h5{margin:0;}</style></head><body><div class='container'><h4 class='center-align'>Wi-Fiのアクセスポイント設定</h4>";
  
  // フォーム開始
  html += "<form action='/save' method='POST'>";

  // プルダウンリストの作成
  html += "<div class='wifi-card'><div class='wifi-header'><h5>SSIDを選択してください</h5></div>";
  html += "<div class='input-field'><select id='wifiSSID' name='ssid' required>";
  html += "<option value='' disabled selected> SSIDを選択</option>";

  
  // サンプルWi-Fiリストを追加
  for (int i = 1; i <= cnt; i++) {
    uint8_t bssid[6];
    WiFi.BSSID(i, bssid);

    html += "<option value='";
    html += String(WiFi.SSID(i));
    html += "'>";
    html += String(WiFi.SSID(i));
    html += "</option>";
  }

  html += "</select></div>";
  html += "<button type='button' class='btn waves-effect waves-light' onclick='location.reload()'>SSIDを再読み込み</button></div>";
  
  // パスワード入力
  html += "<div class='wifi-card'><div class='wifi-header'><h5>パスワードを入力してください</h5></div>";
  html += "<div class='input-field'><input type='password' id='password' name='password' required>";
  html += "<label for='password'></label></div></div>";
  
  // Connectボタン
  html += "<button type='submit' class='btn waves-effect waves-light'>設定</button>";
  html += "</form>"; // フォーム終了

  html += "</div><script src='https://cdnjs.cloudflare.com/ajax/libs/materialize/1.0.0/js/materialize.min.js'></script>";
  html += "<script>document.addEventListener('DOMContentLoaded', function() { var elems = document.querySelectorAll('select'); var instances = M.FormSelect.init(elems); });</script>";
  html += "</body></html>";

  connectWifiServer->send(200, "text/html", html);
}

void ConnectWifi::handleSave() {
  String ssid = connectWifiServer->arg("ssid");
  String password = connectWifiServer->arg("password");

  if (saveWiFiSettings(ssid.c_str(),password.c_str())) {
    connectWifiServer->send(200, "text/plain", "Saved");
    rp2040.reboot();
  } else {
    connectWifiServer->send(200, "text/plain", "Save error");
  }
}

void ConnectWifi::wifiSettingMode(){
  connectWifiServer = new WebServer(_wifi_setting_port);
  _dnsServer = new DNSServer();
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(*_wifi_setting_ip, *_wifi_setting_ip, *_wifi_setting_subnet);
  WiFi.softAP(ssid.c_str(), password.c_str());
  _dnsServer->start(DNS_PORT, "*", *_wifi_setting_ip);

  connectWifiServer->onNotFound(captivePortal);
  connectWifiServer->on("/", handleRoot);
  connectWifiServer->on("/save", handleSave);
  connectWifiServer->begin();
  while(1){
    _dnsServer->processNextRequest();
    connectWifiServer->handleClient();
  }
}

void ConnectWifi::connect(const char *ssid, const char *pass){
  WiFi.mode(WIFI_STA);
  _multi->addAP(ssid, pass);
  do {
    if(_status_pin != -1){
      digitalWrite(_status_pin, !digitalRead(_status_pin));
    }
    delay(100);
    if(_wifi_setting){
      if(BOOTSEL){
        WiFi.mode(WIFI_OFF);
        if(_status_pin != -1){
          digitalWrite(_status_pin, HIGH);
        }
        wifiSettingMode();
      }
    }
  } while (_multi->run() != WL_CONNECTED);
  connected = true;
}

bool ConnectWifi::loadWiFiSettings(char* ssid,char* passoword){
  int read_pointer = 0;
  for(int i=0 ; read_pointer<EEPROM.length() && i<33 ; i++,read_pointer++){
    ssid[i] = EEPROM.read((const int)read_pointer);
    if(ssid[i] == '\0'){
      read_pointer++;
      break;
    }
  }
  if(!isValidSSID(ssid)){
    return false;
  }
  for(int i=0 ; read_pointer<EEPROM.length() && i<64 ; i++,read_pointer++){
    passoword[i] = EEPROM.read((const int)read_pointer);
    if(passoword[i] == '\0'){
      break;
    }
  }
  if(!isValidPassword(passoword)){
    return false;
  }

  return true;
}

bool ConnectWifi::begin(){
  if(!rp2040.isPicoW()){
    return false;
  }
  _multi = new WiFiMulti();

  if(_status_pin != -1){
    pinMode(_status_pin, OUTPUT);
  }

  if(_ssid != nullptr && _password != nullptr){
    connect(_ssid->c_str(),_password->c_str());
  } else {  
    char saved_ssid[33];
    char saved_password[64];
    EEPROM.begin(128);
    if(loadWiFiSettings(saved_ssid,saved_password)){
      connect(saved_ssid,saved_password);
      EEPROM.end();
      if(_status_pin != -1){
        digitalWrite(_status_pin, LOW);
      }
    } else {
      if(_status_pin != -1){
        digitalWrite(_status_pin, HIGH);
      }
      wifiSettingMode();
    }
  }
  return true;
}

bool ConnectWifi::isConnected(){
  return connected && _multi != nullptr && _multi->run() == WL_CONNECTED;
}

String ConnectWifi::getIp(){
  return isConnected() ? WiFi.localIP().toString() : "0.0.0.0";
}