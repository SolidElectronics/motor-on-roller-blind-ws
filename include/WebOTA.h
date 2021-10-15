#include <Arduino.h>
#include <ESP8266WebServer.h>

class WebOTA
{
public:
  WebOTA();
  void setup(ESP8266WiFiClass *pWiFiClient);
  void run();

private:
  ESP8266WebServer m_server;
  ESP8266WiFiClass *m_pWiFiClient;
};