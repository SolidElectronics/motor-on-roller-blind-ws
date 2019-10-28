#include "Arduino.h"
#include "NidayandHelper.h"

NidayandHelper::NidayandHelper(){
  this->_configfile = "/config.json";
}

boolean NidayandHelper::loadconfig(){
  File configFile = SPIFFS.open(this->_configfile, "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<200> jsonBuffer;
  this->_config = jsonBuffer.parseObject(buf.get());

  if (!this->_config.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }
  return true;
}

JsonVariant NidayandHelper::getconfig(){
  return this->_config;
}

boolean NidayandHelper::saveconfig(JsonVariant json){
  File configFile = SPIFFS.open(this->_configfile, "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  json.printTo(configFile);

  Serial.println("Saved JSON to SPIFFS");
  json.printTo(Serial);
  Serial.println();
  return true;
}

void NidayandHelper::resetsettings(WiFiManager& wifim){
  SPIFFS.format();
  wifim.resetSettings();
}
