#ifndef ESPMQTTHelper_H
#define ESPMQTTHelper_H

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <mDNSResolver.h>
#include <ArduinoJson.h>
#include "FS.h"
#include <ESP8266MQTTClient.h>


class ESPMQTTHelper
{

  public:

    ESPMQTTHelper(mDNSResolver::Resolver* mDNSResolver) {
      this->mDNSResolver = mDNSResolver;
      mqttClient =std::unique_ptr<MQTTClient>(new MQTTClient());
    }

    void setup(std::function<void(String topic, String data, bool isDataContinuation)>);
    void loop();
    void sendMessage(String payload);
    void sendMessage(String topic, String payload);

  private:

    mDNSResolver::Resolver* mDNSResolver;
    std::unique_ptr<MQTTClient> mqttClient;

    String wifi_ssid = "";
    String wifi_password = "";
    String mqtt_server = "";
    int mqtt_server_port = 1883;
    String mqtt_user_name = "";
    String mqtt_user_password = "";
    String mqtt_client_name = "";
    String mqtt_input_topic = "";
    String mqtt_output_topic = "";
    const char* config_file = "/config.json";

    // SHA1 fingerprint of the certificate
    String mqtt_server_fingerprint = "";

    IPAddress mqtt_server_ip = INADDR_NONE;

    void setup_wifi();
    void setup_mDNS();
    void resolve_mqtt_server_hostname();
    void setup_mqtt_details(std::function<void(String topic, String data, bool isDataContinuation)>);
    void reconnect();
    bool loadConfigFile();
    void readField(JsonObject *json, char* field_name, String &storeVariable);
    void readField(JsonObject *json, char* field_name, int &storeVariable) ;

};

#endif
