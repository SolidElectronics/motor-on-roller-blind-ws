#include "WebOTA.h"
#include "WiFiUdp.h"

const char* m_serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

ESP8266WebServer* pServer;

WebOTA::WebOTA()
: m_server(82)
, m_pWiFiClient(nullptr)
{
    pServer = &m_server;
}

void WebOTA::setup(ESP8266WiFiClass* pWiFiClient) {
  m_pWiFiClient = pWiFiClient;
  
  /*** Most here is borrowed from Arduino example sketch "WebUpdate" ***/
  // wait for WiFi connection
  if (m_pWiFiClient->status() == WL_CONNECTED) {
    pServer->on("/", HTTP_GET, []() {
      pServer->sendHeader("Connection", "close");
      pServer->send(200, "text/html", m_serverIndex);
    });
    pServer->on("/update", HTTP_POST, []() {
      pServer->sendHeader("Connection", "close");
      pServer->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    }, []() {
      HTTPUpload& upload = pServer->upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.setDebugOutput(true);
        WiFiUDP::stopAll();
        Serial.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace)) { //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      }
      yield();
    });
    pServer->begin();

  } else {
    Serial.println("WiFi Failed");
  }
}

void WebOTA::run()
{
    pServer->handleClient();
}