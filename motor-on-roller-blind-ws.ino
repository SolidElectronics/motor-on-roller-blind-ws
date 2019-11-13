#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <WiFiClient.h>

// philsson
#include "blind.h"
#include "config.h"
#include "mymqtt.h"
#include "utilities.h"
#include "webserver.h"

// Comment in to use reset button. 
// OBS! Will not work properly if motor uses D1-D4 (One of the IOs collide)
// #define USE_RESET_BTN

// Comment in to reset configuration
// For manual triggering. Restore after usage (Requires reflashing before and after)
// #define RESET_CONFIG

using namespace ::philsson::blind;
using namespace ::philsson::config;
using namespace ::philsson::mqtt;
using namespace ::philsson::utilities;

//--------------- CHANGE PARAMETERS ------------------
//Configure Default Settings for Access Point logon
String APid = "BlindsConnectAP";    //Name of access point
String APpw = "nidayand";           //Hardcoded password for access point

//----------------------------------------------------

String version = "1.3.1"; // Not relevant with this Fork (Will not follow master)

// WiFi and Mqtt
WiFiClient espClient;

// Configuration in web interface
// Connect or fallback to AP
WiFiManager wifiManager;

// Publisher and subscriber used by Mqtt
PubSubClient psClient(espClient);

// WebSockets will respond on port 81
WebSocketsServer webSocket = WebSocketsServer(81); 

// Philsson components
MyMqtt myMqtt;
ConfigManager configManager;
Blind blind;
WebServer& webServer = WebServer::instance();

#ifdef USE_RESET_BTN
  ResetButton resetButton; // Not usable with motor on D1-D4
#endif


//! Callback function to be called when the button is pressed.
void onButtonReset() 
{
    Serial.println("Reset has been triggered by button!");
    configManager.reset();
}

//! Position update (position and target position) to webserver and mqtt
//! @param currentPosition Position in % [0, 100]
//! @param targetPosition Target position in % [0, 100]
//! @param clientNum Client ID. Calculated by the web server. 0 if localhost
void sendPosUpdate(int currentPosition, int targetPosition, uint8_t clientNum)
{
  String pubString = "{ \"set\":"+String(targetPosition)+", \"position\":"+String(currentPosition)+" }";
  myMqtt.publish(pubString);
  webSocket.sendTXT(clientNum, pubString);
}
void sendPosUpdate(int currentPosition, int targetPosition)
{
  sendPosUpdate(currentPosition, targetPosition, 0);
}
void sendPosUpdate(uint8_t clientNum = 0)
{
  sendPosUpdate(blind.getPosition(), blind.getTargetPosition(), clientNum);
}

//! Save blind state to SPIFFS only if data is new
void saveBlindState()
{
  static long oldCheck = 0;

  long newCheck = blind.getStep() * blind.getMaxStep() + blind.getInverted();

  if (newCheck != oldCheck)
  {
  configManager.getConfig().blindPos = blind.getStep();
  configManager.getConfig().blindMaxPos = blind.getMaxStep();
  configManager.getConfig().directionInverted = blind.getInverted();
  configManager.saveConfig();
  }
  oldCheck = newCheck;
}

//! Sets the blind state. Called after a restart when positioning 
//! and calibration has been lost
void updateBlindState()
{
  blind.correctData(configManager.getConfig().blindPos,
                    configManager.getConfig().blindMaxPos,
                    configManager.getConfig().directionInverted);
}

//! Process received messages. Will handle messages both from webserver or mqtt
//! @param msg Command to be ran
//! @param clientNum The client sending this. 0 if the ESP itself.
//!                  Increments for each connected client
void processMsg(String msg, uint8_t clientNum)
{
  if (msg == "(0)")
  {
    blind.stop();
    saveBlindState();
  }
  else if (msg == "(start)")
  {
    blind.setOpen();
  }
  else if (msg == "(max)")
  {
    blind.setClosed();
    saveBlindState();
  }
  else if (msg == "(1)") // TODO: Replace with Up or Down?
  {
    // Move down without limit
    blind.moveDown();
  }
  else if (msg ==  "(-1)")
  {
    // Move up without limit
    blind.moveUp();
  }
  else if (msg == "(update)")
  {
    //S end position details to client
    sendPosUpdate(clientNum);
  }
  else if (msg == "(save)")
  {
    saveBlindState();
  }
  else // TODO: fault tolerant else
  {
    /*
     * Any other message will take the blind to a position
     *  Incoming value = 0-100
     *  path is now the position
    */
    blind.setPosition(msg.toInt());
    sendPosUpdate(clientNum);
    // Saving the blind state is handled by callback and not handled here
  }
}

//! Handle messages from the web server
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  switch(type) 
  {
  case WStype_TEXT:
    Serial.printf("Webclient nr [%u] sent: %s\n", num, payload);

    String res = (char*)payload;

    //Send to common MQTT and websocket function
    processMsg(res, num);
    break;
  }
}

//! Handle MQTT messages
void mqttCallback(char* topic, byte* payload, unsigned int length) 
{
  Serial.printf("Message arrived [%s]\n", topic);
  String res = "";
  for (int i = 0; i < length; i++) 
  {
    res += String((char) payload[i]);
  }
  processMsg(res, NULL);
}

//! Called if something went wrong at startup
void waitAndReboot()
{
  Serial.println("Scheduling reboot...");
  delay(10e4);
  ESP.restart();
}

void setup(void)
{
  // Make sure motor driver is at rest
  blind.restCoils();
  
  Serial.begin(115200);
  delay(500);
  Serial.println("Starting now...");

  #ifdef USE_RESET_BTN
   resetButton.setup(onButtonReset);
  #endif

  #ifdef RESET_CONFIG
    configManager.reset();
  #endif


  /*********************** Configuration Manager *********************/
  {
    // Init SPIFFS
    if (!configManager.init())
    {
       waitAndReboot();
    }
    // If we cannot load config (corrupted) we overwrite it
    if (configManager.loadConfig())
    {
      updateBlindState(); // TODO: Register this as a callback instead
    }
    else
    {
      configManager.saveConfig();
    }
  }
  /*******************************************************************/



  /********************** Network Connection Stuff *******************/
  {
    // Set the WIFI hostname
    WiFi.hostname(configManager.getConfig().name);

    // Attach our configuration to the wifi manager
    configManager.connectConfigToWifiManager(wifiManager);

    // Set the WiFi SSID and passwd to use if in AP mode
    wifiManager.autoConnect(APid.c_str(), APpw.c_str());
    wifiManager.setSaveConfigCallback(setNeedSaving);
  
    // Setup multi DNS (Bonjour)
    if (MDNS.begin(configManager.getConfig().name)) 
    {
      Serial.println("MDNS responder started");
      MDNS.addService("http", "tcp", 80);
      MDNS.addService("ws", "tcp", 81);
    } 
    else 
    {
      Serial.println("Error setting up MDNS responder!");
      waitAndReboot(); 
    }
    Serial.print("Connect to http://"+String(configManager.getConfig().name)+".local or http://");
    Serial.println(WiFi.localIP());
  }
  /*******************************************************************/
 


  /**************************** MQTT *********************************/
  myMqtt.setPubSubClient(psClient);
  if (String(configManager.getConfig().mqttServer) != "")
  {
    Serial.println("Registering MQTT server");
    myMqtt.setServer(configManager.getConfig().mqttServer, 
                     configManager.getConfig().getmqttPort().toInt());
    myMqtt.setClientID(String(configManager.getConfig().name));
    myMqtt.setUID(configManager.getConfig().mqttUID);
    myMqtt.setPassword(configManager.getConfig().mqttPWD);
    // Register callback on received mqtt command
    Serial.println("Registering mqtt callback");
    myMqtt.setCallback(mqttCallback);
  }
  /*******************************************************************/
 


  /************************** Web Server *****************************/
  //Start HTTP server
  webServer.setup();
  webServer.begin();
  Serial.println("HTTP Server Started");
  //Start websocket
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("Websockets are set up");
  //Update webpage
  Serial.println("Settings up web page");
  webServer.updatePage(version, String(configManager.getConfig().name));
  /*******************************************************************/
 


  /*************************** OTA Setup *****************************/
  {
    // Authentication to avoid unauthorized updates
    // ArduinoOTA.setPassword(OTA_PWD);
    Serial.println("Setting up OTA");

    ArduinoOTA.setHostname(configManager.getConfig().name);

    ArduinoOTA.onStart([]() {
      Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    Serial.println("OTA Setup completed");
  }
  /*******************************************************************/



  /*************************** Blind Settings ************************/
  updateBlindState();
  sendPosUpdate();
  blind.setPosUpdateCallback(sendPosUpdate);
  blind.setReachedTargetCallback(saveBlindState);
  Serial.printf("Maxpos %d\n", configManager.getConfig().blindMaxPos);
  /*******************************************************************/



  // Make sure motor driver is at rest
  blind.restCoils();
  Serial.println("Driver coils turned off");
}

void loop(void)
{
  // OTA client code
  ArduinoOTA.handle();

  // Handle onboard button press
  #ifdef USE_RESET_BTN
    resetButton.read();
  #endif

  // Websocks listener
  webSocket.loop();

  // Wifi
  // TODO: Revise solution (Not tested)
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi connection lost. Trying to reconnect...");
    WiFi.reconnect();
  }

  // Serving the webpage
  webServer.handleClient();

  myMqtt.run();

  blind.run();

  //! If config needs saving. E.g. WifiManager changes
  configManager.run();
}
