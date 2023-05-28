#ifndef ConnectWifi_h
#define ConnectWifi_h

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <DNSServer.h>

#define WIFI_SETTING_SSID "pico"                      // Wi-Fi設定用のSSID
#define WIFI_SETTING_PASSWORD "picopico"              // Wi-Fi設定用のパスワード
#define DEFAULT_WIFI_SETTING_IP "192.168.10.2"
#define DEFAULT_WIFI_SETTING_SUBNET "255.255.255.0"

class ConnectWifi {
public:
    ConnectWifi(const char* wifi_setting_ip=DEFAULT_WIFI_SETTING_IP,const char* wifi_setting_subnet=DEFAULT_WIFI_SETTING_SUBNET,const int wifi_setting_port=80,const int status_pin=LED_BUILTIN);
    ConnectWifi(const bool wifi_setting,const char* ssid,const char* password,const int status_pin=LED_BUILTIN);
    bool begin();
    bool isConnected();
    String getIp();
private:
    const String ssid = WIFI_SETTING_SSID;
    const String password = WIFI_SETTING_PASSWORD;
    WiFiMulti *_multi = nullptr;
    DNSServer *_dnsServer = nullptr;
    IPAddress *_wifi_setting_ip = nullptr;
    IPAddress *_wifi_setting_subnet = nullptr;
    String *_ssid = nullptr;
    String *_password = nullptr;
    int _wifi_setting_port;
    bool _wifi_setting;
    int _status_pin;
    const byte DNS_PORT = 53;
    bool connected = false;

    static bool isValidSSID(const char* ssid);
    static bool isValidPassword(const char* password);
    static bool saveWiFiSettings(const char* ssid,const char* passoword);
    static void captivePortal();
    static void handleRoot();
    static void handleSave();

    void wifiSettingMode();
    bool loadWiFiSettings(char* ssid,char* passoword);
    void connect(const char *ssid, const char *pass);
    

};

static WebServer *connectWifiServer;

#endif