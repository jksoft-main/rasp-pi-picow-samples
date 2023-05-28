#include "AmbientRspPico.h"

const String AMBIENT_URL = "https://ambidata.io";
const String ambient_keys[] = {"\"d1\":\"", "\"d2\":\"", "\"d3\":\"", "\"d4\":\"", "\"d5\":\"", "\"d6\":\"", "\"d7\":\"", "\"d8\":\""};

Ambient::Ambient() {
}

bool Ambient::begin(unsigned int channelId, const char * writeKey) {
  this->channelId = channelId;

  if (sizeof(writeKey) > AMBIENT_WRITEKEY_SIZE) {
    return false;
  }
  strcpy(this->writeKey, writeKey);

  this->lastsend = 0;

  return true;
}

bool Ambient::set(int field,const char * data) {
  --field;
  if (field < 0 || field >= AMBIENT_NUM_PARAMS) {
    return false;
  }
  if (strlen(data) > AMBIENT_DATA_SIZE) {
    return false;
  }
  this->data[field].set = true;
  strcpy(this->data[field].item, data);

  return true;
}

bool Ambient::set(int field, double data)
{
	return set(field,String(data).c_str());
}

bool Ambient::set(int field, float data)
{
	return set(field,String(data).c_str());
}

bool Ambient::set(int field, int data)
{
	return set(field, String(data).c_str());
}

bool Ambient::send() {
  if (this->lastsend != 0 && (millis() - this->lastsend) < 4999) {
    this->status = 403;
    return false;
  }

  HTTPClient http;
  http.setInsecure();

  String url = AMBIENT_URL + "/api/v2/channels/" + this->channelId + "/data";
  http.begin(url.c_str());
  http.addHeader("Content-Type", "application/json");

  String body = "{\"writeKey\":\"" + String(this->writeKey) + "\",";

  for (int i = 0; i < AMBIENT_NUM_PARAMS; i++) {
    if (this->data[i].set) {
      body += ambient_keys[i];
      body += this->data[i].item;
      body += "\",";
    }
  }
  body.remove(body.length() - 1);
  body += "}\r\n";

  this->status = http.POST(body.c_str());
  http.end();

  if (this->status == HTTP_CODE_OK) {
    return true;
  }    

  return false;
}