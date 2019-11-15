#pragma once

#include <WiFiManager.h> // Not used to store but at runtime
#include <ArduinoJson.h>


namespace philsson {
namespace config {

namespace {
//! Getter and Setter for char* using String
#define GETSET(x) \
  String get##x() \
    { return String(x); } \
  void set##x(String y) \
    { y.toCharArray(x, sizeof(x)); }
}

void setNeedSaving();

struct Config
{
    // All these settings are stored in json format on SPIFFS

    // WifiManager Settings
    // These need to be char arrays
    char name[20];
    char mqttServer[20];
    char mqttPort[10];
    char mqttUID[10];
    char mqttPWD[10];

    // Settings outside of WifiManager
    long blindPos;
    long blindMaxPos;
    bool directionInverted;
    uint8_t speedUp;
    uint8_t speedDown;

    GETSET(name);
    GETSET(mqttServer);
    GETSET(mqttPort);
    GETSET(mqttUID);
    GETSET(mqttPWD);
};

class ConfigManager
{
public:
    ConfigManager();

    bool init();

    //! Do nescessary tasks. E.g save config if needed
    void run();

    bool saveConfig();

    //! Saves config if needed
    bool saveCheckWifiManager();

    //! Load the config from SPIFFS
    bool loadConfig();

    bool printConfig();

    bool reset();

    Config& getConfig();

    //! Returns True if changes were done since last save
    bool configChanged();

    void connectConfigToWifiManager(WiFiManager &wifiManager);

    static bool needSaving;

private:

    Config m_config;
    JsonVariant m_jsonConfig;
    String m_configFile;

    bool m_configSaved;

    //Define customer parameters for WIFI Manager
    WiFiManagerParameter m_wmName;
    WiFiManagerParameter m_wmMqttServer;
    WiFiManagerParameter m_wmMqttPort;
    WiFiManagerParameter m_wmMqttUID;
    WiFiManagerParameter m_wmMqttPWD;
    WiFiManagerParameter m_wmText;
    WiFiManagerParameter m_wmText2;
};

} // namespace config
} // namespace philsson