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

    void setPubSubClient(PubSubClient* pWifiClient);

    void setServer(const char * domain, uint16_t port);

    void setCallback(MQTT_CALLBACK_SIGNATURE);

    //! Do the stuff
    void run();

    void reconnect();
    void reconnect(std::list<const char*> topics);
    void reconnect(String uid, String pwd);
    void reconnect(String uid, String pwd, std::list<const char*> topics);

    void publish(String topic, String payload);

    //! This bool will be flipped once every 60s
    static bool publishStatePending;

private:

    void setTopics();

    // The MQTT client
    PubSubClient* m_pPsclient;

    String m_topicIn;
    String m_topicOut;

    String m_mqttClientId;

    Ticker m_mqttHeartbeatTicker;
};