#ifndef ESP8266MQTTCLIENT_H
#define ESP8266MQTTCLIENT_H

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <mDNSResolver.h>
#include <ArduinoJson.h>
#include "FS.h"
#include <ArduinoOTA.h>


class ESP8266MQTTClient
{

  public:

    ESP8266MQTTClient(WiFiClientSecure* secureClient, PubSubClient pubSubClient, mDNSResolver::Resolver* mDNSResolver) {
      this->pubSubClient = pubSubClient;
      this->secureClient = secureClient;
      this->mDNSResolver = mDNSResolver;
    }

    void setup(MQTT_CALLBACK_SIGNATURE);
    void loop();
    void sendMessage(const char* payload);
    void sendMessage(const char* topic, const char* payload);

  private:

    mDNSResolver::Resolver* mDNSResolver;
    PubSubClient pubSubClient;
    WiFiClientSecure* secureClient;


    String wifi_ssid = "";
    String wifi_password = "";
    String mqtt_server = "";
    int mqtt_server_port = 1883;
    String mqtt_user_name = "";
    String mqtt_user_password = "";
    String mqtt_client_name = "";
    String mqtt_input_topic = "";
    String mqtt_output_topic = "";
    int ota_port = 8266;
    String ota_password = "";
    String ota_hostname = "";

    const char* config_file = "/config.json";

    // SHA1 fingerprint of the certificate
    String mqtt_server_fingerprint = "";

    IPAddress mqtt_server_ip = INADDR_NONE;

    void setup_wifi();
    void setup_mDNS();
    void setup_OTA();
    void resolve_mqtt_server_hostname();
    void setup_mqtt_details(MQTT_CALLBACK_SIGNATURE);
    void reconnect();
    bool loadConfigFile();
    void readField(JsonObject *json, char* field_name, String &storeVariable);
    void readField(JsonObject *json, char* field_name, int &storeVariable) ;

};

#endif
