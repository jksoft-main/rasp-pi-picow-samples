#include <WiFi.h>
#include <WebServer.h>
#include <uri/UriBraces.h>
#include <LEAmDNS.h>
#include "ConnectWifi.h"

#define DOMEIN_NAME "picow"

ConnectWifi connect;
WebServer server(80);

void handleRoot() {
  float temperature = analogReadTemp();
  String html = "<!DOCTYPE html><html lang='ja'>";
  html += "<head><meta charset='UTF-8' />";
  html += "<meta name='viewport' ";
  html += "content='width=device-width,";
  html += "initial-scale=1.0' />";
  html += "<title>LED Button</title>";
  html += "<style>input {width: 100%; font-size: 200%;}";
  html += "</style></head>";
  html += "<body><h1>LEDの状態を変更します</h1>";
  html += "<form action='./led/on'>";
  html += "<input type='submit' value='LED ON' /></form>";
  html += "<form action='./led/off'>";
  html += "<input type='submit' value='LED OFF' /></form>";
  html += "<p>Board temperature:";
  html += String(temperature) + "</p>";
  server.send(200, "text/html", html);
}

void redirectHome() {
  server.sendHeader("Location",
    String("http://" DOMEIN_NAME ".local"));
  server.send(302, "text/plain", "");
  server.client().stop();
}

void handleLed() {
  String status = server.pathArg(0);
  if(status == "on"){
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
  redirectHome();
}

void setup() {
  Serial.begin(115200);
  connect.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  MDNS.begin(DOMEIN_NAME);
  server.on("/", handleRoot);
  server.on(UriBraces("/led/{}"), handleLed);
  server.onNotFound(redirectHome);
  server.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.println("HTTP server started");
}

void loop() {
  MDNS.update();
  server.handleClient();
}
