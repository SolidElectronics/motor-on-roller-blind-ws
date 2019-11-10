#pragma once

// Std
#include <list>
// Mqtt
#include <PubSubClient.h>
// Timers
#include <Ticker.h>
// Wifi. For retrieval of local IP
#include <WiFiManager.h>

class MyMqtt {
public:
MyMqtt();

  void setPubSubClient(PubSubClient& wifiClient);

  void setServer(const char * domain, uint16_t port);

  void setClientID(String clientID);

  void setUID(String userID);

  void setPassword(String password);

  //! Callback to be used when a msg is received
  void setCallback(MQTT_CALLBACK_SIGNATURE);

  //! Do the stuff
  void run();

  //! Reconnect without authentication
  void reconnect();

  //! Reconnect without authentication
  //! Subscribe to some topics
  //! @param topics Topics to subscribe to
  void reconnect(std::list<const char*> topics);

  //! Reconnect with authentication
  //! @param uid User ID
  //! @param pwd User Password
  void reconnect(String uid, String pwd);

  //! Reconnect with authentication
  //! Subscribe to some topics
  //! @param uid User ID
  //! @param pwd User Password
  //! @param topics Topics to subscribe to
  void reconnect(String uid, String pwd, std::list<const char*> topics);

  void publish(String topic, String payload);
  void publish(String payload);

  //! This bool will be flipped once every 60s
  static bool publishStatePending;

private:

  void setTopics();

  // The MQTT client
  PubSubClient* m_pPsclient;
    
  String m_topicIn;
  String m_topicOut;

  String m_clientId;
  String m_userId;
  String m_password;

  // Used for timed republishing. Doubles as availability
  String m_lastMsg;

  // Timer to trigger internal callback
  Ticker m_heartbeatTicker;
};