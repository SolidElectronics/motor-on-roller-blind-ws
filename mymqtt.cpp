#include "mymqtt.h"

// Wifi. For retrieval of local IP
#include <WiFiManager.h>

namespace philsson {
namespace mqtt {
  
bool MyMqtt::publishStatePending = false;

namespace {
  void setPublishState()
  {
    MyMqtt::publishStatePending = true;
  }
}

MyMqtt::MyMqtt()
: m_pPsclient(0)
, m_baseTopic()
, m_topicIn()
, m_topicOut()
, m_clientId(String("EspClient-" + String(ESP.getChipId())))
, m_userId()
, m_password()
, m_lastMsg()
, m_heartbeatTicker()
, m_serverSet(false)
{
    setTopics();
    
    m_heartbeatTicker.attach(60, setPublishState);
}

void MyMqtt::setPubSubClient(PubSubClient& wifiClient)
{
  m_pPsclient = &wifiClient;
}

void MyMqtt::setServer(const char * domain, uint16_t port)
{
  if (!m_pPsclient)
  {
    Serial.println("m_pPsClient = NULL");
    return;
  }
  m_pPsclient->setServer(domain, port);
  m_serverSet = true;
}

void MyMqtt::setClientID(String clientID)
{
  m_clientId = clientID;
  setTopics();
}

void MyMqtt::setUID(String userID)
{
  m_userId = userID;
}

void MyMqtt::setPassword(String password)
{
  m_password = password;
}

void MyMqtt::setCallback(MQTT_CALLBACK_SIGNATURE)
{
  if (!m_pPsclient)
  {
    Serial.println("m_pPsClient = NULL");
    return;
  }
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
  mylist.push_back(m_topicIn.c_str());
  reconnect(uid, pwd, mylist);
}

void MyMqtt::reconnect(String uid, String pwd, std::list<const char*> topics)
{
  if (!m_pPsclient)
  {
    Serial.println("m_pPsClient = NULL");
    return;
  }
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
    if ((mqttLogon ? m_pPsclient->connect(m_clientId.c_str(), uid.c_str(), pwd.c_str())
                   : m_pPsclient->connect(m_clientId.c_str()))) 
    {
      Serial.println("connected");

      //Send register MQTT message with JSON of chipid and ip-address
      publish(m_baseTopic + "register", "{ \"id\": \"" + String(ESP.getChipId()) + "\", \"ip\":\"" + WiFi.localIP().toString() +"\"}");

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

void MyMqtt::publish(String topic, String payload)
{
  m_lastMsg = payload;
  if (!m_pPsclient)
  {
    Serial.println("m_pPsClient = NULL");
    return;
  }

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

void MyMqtt::publish(String payload)
{
  publish(m_topicOut, payload);
}

void MyMqtt::run()
{
  if (!m_serverSet)
  {
    return;
  }
  
  // Reconnect if needed
  reconnect(m_userId.c_str(), m_password.c_str());

  // State Publish if needed
  if (publishStatePending)
  {
    Serial.println("Mqtt ticker...");
    MyMqtt::publishStatePending = false;
    publish(m_lastMsg);
  }
}

void MyMqtt::setTopics()
{
  m_baseTopic = "/blind/" + m_clientId + "/";
  m_topicIn = m_baseTopic + "in";
  m_topicOut = m_baseTopic + "out";
}

} // namespace philsson
} // namespace mqtt
