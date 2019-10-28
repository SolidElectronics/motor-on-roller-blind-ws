#ifndef NidayandHelper_h
#define NidayandHelper_h

#include "Arduino.h"
#include <ArduinoJson.h>
#include "FS.h"
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
#include <list>

class NidayandHelper {
  public:
    NidayandHelper();
    boolean loadconfig();
    JsonVariant getconfig();
    boolean saveconfig(JsonVariant json);
    void resetsettings(WiFiManager& wifim);

  private:
    JsonVariant _config;
    String _configfile;
};

#endif
