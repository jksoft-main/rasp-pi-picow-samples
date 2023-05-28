#ifndef AmbientRspPico_h
#define AmbientRspPico_h

#include "Arduino.h"
#include <WiFi.h>
#include <HTTPClient.h>

#define AMBIENT_WRITEKEY_SIZE 18
#define AMBIENT_DATA_SIZE 24
#define AMBIENT_NUM_PARAMS 8

class Ambient
{
public:
  Ambient(void);
  bool begin(unsigned int channelId, const char * writeKey);
  bool set(int field,const char * data);
	bool set(int field, double data);
	bool set(int field, float data);
	bool set(int field, int data);
  bool send();
  int status;
    
private:
  unsigned int channelId;
  char writeKey[AMBIENT_WRITEKEY_SIZE];
  unsigned long lastsend;
  struct {
    int set;
    char item[AMBIENT_DATA_SIZE];
  } data[AMBIENT_NUM_PARAMS];
};

#endif