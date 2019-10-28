#include "mymqtt.h"

bool MyMqtt::publishStatePending = false;

namespace {
    void setPublishState()
    {
        MyMqtt::publishStatePending = true;
    }
}

MyMqtt::MyMqtt()
: m_pPsclient(0)
, m_topicIn()
, m_topicOut()
, m_mqttClientId("ESPClient-" + String(ESP.getChipId()))
, m_mqttHeartbeatTicker()
{
    setTopics();
    
    m_mqttHeartbeatTicker.attach(60, setPublishState);

}

void MyMqtt::setPubSubClient(PubSubClient* pWifiClient)
{
    m_pPsclient = pWifiClient;
}

void MyMqtt::setServer(const char * domain, uint16_t port)
{
    m_pPsclient->setServer(domain, port);
}

void MyMqtt::setCallback(MQTT_CALLBACK_SIGNATURE)
{
    m_pPsclient->setCallback(callback);
}

void MyMqtt::reconnect()
{
  reconnect(String(NULL), String(NULL));
}
void MyMqtt::reconnect(std::list<const char*> topics)
{
  reconnect(String(NULL), String(NULL), topics);
}
void MyMqtt::reconnect(String uid, String pwd)
{
  std::list<const char*> mylist;
  reconnect(uid, pwd, mylist);
}

void MyMqtt::reconnect(String uid, String pwd, std::list<const char*> topics){
  // Loop until we're reconnected
  boolean mqttLogon = false;
  if (uid!=NULL and pwd != NULL)
  {
    mqttLogon = true;
  }

  while (!m_pPsclient->connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if ((mqttLogon ? m_pPsclient->connect(m_mqttClientId.c_str(), uid.c_str(), pwd.c_str())
                   : m_pPsclient->connect(m_mqttClientId.c_str()))) 
    {
      Serial.println("connected");

      //Send register MQTT message with JSON of chipid and ip-address
      publish("/raw/esp8266/register", "{ \"id\": \"" + String(ESP.getChipId()) + "\", \"ip\":\"" + WiFi.localIP().toString() +"\"}");

      //Setup subscription
      if (!topics.empty()){
        for (const char* t : topics){
           m_pPsclient->subscribe(t);
           Serial.println("Subscribed to "+String(t));
        }
      }

    } 
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(m_pPsclient->state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      ESP.wdtFeed();
      delay(5000);
    }
  }
  if (m_pPsclient->connected())
  {
    m_pPsclient->loop();
  }
}

void MyMqtt::publish(String topic, String payload){
  Serial.println("Trying to send msg..."+topic+":"+payload);
  //Send status to MQTT bus if connected
  if (m_pPsclient->connected()) 
  {
    m_pPsclient->publish(topic.c_str(), payload.c_str());
  } 
  else 
  {
    Serial.println("PubSub client is not connected...");
  }
}

void MyMqtt::run()
{
    // State Publish if needed
    if (publishStatePending)
    {
        Serial.println("Ticker...");
        MyMqtt::publishStatePending = false;
        // TODO: Publish the topic
    }
}

void MyMqtt::setTopics()
{
    // TODO: If name...
    // else
    String baseTopic = "/raw/esp8266/" + String(ESP.getChipId()) + "/";
    m_topicIn = baseTopic + "in";
    m_topicOut = baseTopic + "out";
}