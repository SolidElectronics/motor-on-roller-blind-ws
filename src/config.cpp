#include "config.h"

#include "Arduino.h"
#include "FS.h"
#include <ESP8266WiFi.h>
#include <SPI.h>

namespace philsson {
namespace config {

bool ConfigManager::needSaving = false;

void setNeedSaving()
{
  ConfigManager::needSaving = true;
}

ConfigManager::ConfigManager()
: m_config()
, m_jsonConfig()
, m_configFile("/config.json")
, m_configSaved()
, m_wmName("name", "Name (MQTT topic)", m_config.name, sizeof(m_config.name))
, m_wmMqttServer("server", "MQTT Server", m_config.mqttServer, sizeof(m_config.mqttServer))
, m_wmMqttPort("port", "MQTT port", m_config.mqttPort, sizeof(m_config.mqttPort))
, m_wmMqttUID("uid", "MQTT username", m_config.mqttUID, sizeof(m_config.mqttUID))
, m_wmMqttPWD("pwd", "MQTT password", m_config.mqttPWD, sizeof(m_config.mqttPWD))
, m_wmText("<p><b>Optional MQTT server parameters:</b></p>")
, m_wmText2("<script>t = document.createElement('div'); \
   t2 = document.createElement('input'); \
   t2.setAttribute('type', 'checkbox'); \
   t2.setAttribute('id', 'tmpcheck'); \
   t2.setAttribute('style', 'width:10%'); \
   t2.setAttribute('onclick', \"if(document.getElementById('Rotation').value == 'false') \
     {document.getElementById('Rotation').value = 'true'} \
     else {document.getElementById('Rotation').value = 'false'}\"); \
   t3 = document.createElement('label');tn = document.createTextNode('Clockwise rotation'); \
   t3.appendChild(t2); \
   t3.appendChild(tn); \
   t.appendChild(t3); \
   document.getElementById('Rotation').style.display='none'; \
   document.getElementById(\"Rotation\").parentNode.insertBefore(t, document.getElementById(\"Rotation\")); \
   </script>")
{
}

void ConfigManager::run()
{
  if (needSaving)
  {
    saveConfig();
    needSaving = false;
  }
}

bool ConfigManager::init()
{
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount file system");
    return false;
  }
  return true;
}

bool ConfigManager::saveConfig()
{
  File file = SPIFFS.open(m_configFile, "w");
  if (!file)
  {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  StaticJsonDocument<512> doc;
  doc["name"] = m_config.name;
  doc["mqttServer"] = m_config.mqttServer;
  doc["mqttPort"] = m_config.mqttPort;
  doc["mqttUID"] = m_config.mqttUID;
  doc["mqttPWD"] = m_config.mqttPWD;
  doc["blindPos"] = m_config.blindPos;
  doc["blindMaxPos"] = m_config.blindMaxPos;
  doc["directionInverted"] = m_config.directionInverted;
  doc["speedUp"] = m_config.speedUp;
  doc["speedDown"] = m_config.speedDown;

  // Write
  if (!serializeJson(doc, file))
  {
    Serial.println("Failed to write config to file");
    return false;
  }

  file.close();

  printConfig();

  Serial.println("Saved JSON to SPIFFS");

  return true;
}

bool ConfigManager::saveCheckWifiManager()
{
  if (needSaving)
  {
    Serial.println("Save triggered at startup");

    strcpy(m_config.name, m_wmName.getValue());
    strcpy(m_config.mqttServer, m_wmMqttServer.getValue());
    strcpy(m_config.mqttPort, m_wmMqttPort.getValue());
    strcpy(m_config.mqttUID, m_wmMqttUID.getValue());
    strcpy(m_config.mqttPWD, m_wmMqttPWD.getValue());

    saveConfig();
    return true;
  }
  Serial.println("No saving needed");
  return false;
}

bool ConfigManager::loadConfig()
{
  bool result = true;

  File file = SPIFFS.open(m_configFile, "r");
  if (!file)
  {
    Serial.println("Failed to open config file");
    result = false;
  }

  StaticJsonDocument<512> doc;
  if (result)
  {
    size_t size = file.size();
    if (size > 1024)
    {
      Serial.println("Config file size is too large");
      result = false;
    }
    else
    {
      printConfig();
    }
  }

  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.println(F("Failed to read file, using default configuration"));
    result = false;
  }

  m_config.setname(doc["name"] | String("EspClient-" + String(ESP.getChipId())));
  m_config.setmqttServer(doc["mqttServer"] | "");
  m_config.setmqttPort(doc["mqttPort"] | "1883");
  m_config.setmqttUID(doc["mqttUID"] | "");
  m_config.setmqttPWD(doc["mqttPWD"] | "");
  m_config.blindPos = doc["blindPos"] | -1;
  m_config.blindMaxPos = doc["blindMaxPos"] | -1;
  m_config.directionInverted = doc["directionInverted"] | false;
  m_config.speedUp = doc["speedUp"] | 5;
  m_config.speedDown = doc["speedDown"] | 5;

  Serial.println("Configuration completed");

  return result;
}

bool ConfigManager::printConfig()
{
  File file = SPIFFS.open(m_configFile, "r");
  if (!file)
  {
    Serial.println("Failed to open config file");
    return false;
  }

  Serial.println("This is the loaded configuration:");
  // Extract each characters by one by one
  while (file.available())
  {
    char c = file.read();
    Serial.print(c);
    if (c == char(','))
    {
      Serial.println();
    }
  }
  Serial.println();

  // Close the file
  file.close();
  return true;
}

bool ConfigManager::reset()
{
  SPIFFS.format();
  return true;
}

Config &ConfigManager::getConfig()
{
  return m_config;
}

void ConfigManager::connectConfigToWifiManager(WiFiManager &wifiManager)
{
  wifiManager.addParameter(&m_wmName);
  wifiManager.addParameter(&m_wmMqttServer);
  wifiManager.addParameter(&m_wmMqttPort);
  wifiManager.addParameter(&m_wmMqttUID);
  wifiManager.addParameter(&m_wmMqttPWD);
  wifiManager.addParameter(&m_wmText);
  wifiManager.addParameter(&m_wmText2);
}

} // namespace config
} // namespace philsson