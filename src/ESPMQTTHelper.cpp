#include "ESPMQTTHelper.h"

using namespace mDNSResolver;

#define DEBUG_ESP_PORT
#define DEBUG_ESP_PORT Serial

#ifdef DEBUG_ESP_PORT
#define LOG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#endif

void ESPMQTTHelper::setup(std::function<void(String topic, String data, bool isDataContinuation)> onData) {
  setup_wifi();
  //setup_mDNS();
  setup_mqtt_details(onData);
}

void ESPMQTTHelper::setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  if (loadConfigFile()) {
    LOG("Config file loaded.\n");
  }
  else {
    LOG("Error loading config file.\n");
  }
  LOG("Connecting to %s\n", wifi_ssid.c_str());

  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    LOG(".");
  }

  LOG("WiFi connected");
  LOG("IP address: %s\n", WiFi.localIP().toString().c_str());
}

void ESPMQTTHelper::setup_mDNS() {
  if (!MDNS.begin(mqtt_client_name.c_str())) {
    // Start the mDNS responder for esp8266.local
    LOG("Error setting up MDNS responder! \n");
  }
  LOG("mDNS responder started \n");
}


 
void ESPMQTTHelper::resolve_mqtt_server_hostname() {
  if (mqtt_server.endsWith(".local")) {
    LOG("resolving using mDNS resolver because local hostname \n");
    
    mDNSResolver->setLocalIP(WiFi.localIP());
    mqtt_server_ip = mDNSResolver->search(mqtt_server.c_str());
    LOG("%s\n", mqtt_server_ip.toString().c_str());
    
  }
  else {
    LOG("resolving using WiFi.hostByName resolver because not local hostname \n");
    if (!WiFi.hostByName(mqtt_server.c_str(), mqtt_server_ip)) {
      LOG("Can't resolve mqtt_server hostname... restarting.. \n");
      ESP.restart();
    }
  }

  if (mqtt_server_ip != INADDR_NONE) {
    LOG("MQTT Server Resolved: %s\n", mqtt_server_ip.toString().c_str());
  } else {
    LOG("Can't resolve mqtt_server hostname. Halt! \n");
    ESP.restart();
  }
}

void ESPMQTTHelper::setup_mqtt_details(std::function<void(String topic, String data, bool isDataContinuation)> onData) {
  resolve_mqtt_server_hostname();
  
  mqttClient->onSecure([&](WiFiClientSecure * client, String host) {
    bool result = client->verify(mqtt_server_fingerprint.c_str(), mqtt_server.c_str());
    if (!result){
      LOG("Server fingerprint verification failed! \n");
    }
    return result;
  });

  //topic, data, data is continuing
  mqttClient->onData(onData);

  mqttClient->onSubscribe([](int sub_id) {
    LOG("Subscribe topic id: %d ok\r\n", sub_id);
  });
  mqttClient->onConnect([&]() {
    LOG("MQTT: Connected\r\n");
    LOG("Subscribe id: %d\r\n", mqttClient->subscribe("inTopic", 2));
  });

  String connectionURL = String("mqtts://")+mqtt_user_name+":"+mqtt_user_password+"@"+mqtt_server_ip.toString()+":"+mqtt_server_port;


  LOG("connection url: %s \n", connectionURL.c_str());
  mqttClient->begin(connectionURL);
}

void ESPMQTTHelper::sendMessage(String payload) {
  mqttClient->publish(mqtt_output_topic.c_str(), payload);
}

void ESPMQTTHelper::sendMessage(String topic, String payload) {
  mqttClient->publish(topic, payload);
}

bool ESPMQTTHelper::loadConfigFile() {
  if (!SPIFFS.begin()) {
    LOG("Failed to mount file system \n");
    return false;
  }
  else {
    LOG("File system mounted. \n");
  }

  File configFile = SPIFFS.open(config_file, "r");
  if (!configFile) {
    LOG("Failed to open config file \n");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    LOG("Config file size is too large \n");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    LOG("Failed to parse config file. \n");
    return false;
  }

  readField(&json, "wifi_ssid", wifi_ssid);
  readField(&json, "wifi_password", wifi_password);
  
  readField(&json, "mqtt_server", mqtt_server);
  readField(&json, "mqtt_server_port", mqtt_server_port);
  readField(&json, "mqtt_user_name", mqtt_user_name);
  readField(&json, "mqtt_user_password", mqtt_user_password);
  readField(&json, "mqtt_server_fingerprint", mqtt_server_fingerprint);
  readField(&json, "mqtt_client_name", mqtt_client_name);
  
  readField(&json, "mqtt_input_topic", mqtt_input_topic);
  readField(&json, "mqtt_output_topic", mqtt_output_topic);
  
  return true;
}

void ESPMQTTHelper::readField(JsonObject *json, char* field_name, String &storeVariable) {
  if (json->containsKey(field_name)) {
    const char* field_value = (*json)[field_name];
    storeVariable = String(field_value);
  }
}

void ESPMQTTHelper::readField(JsonObject *json, char* field_name, int &storeVariable) {
  if (json->containsKey(field_name)) {
    storeVariable = (*json)[field_name];
  }
}

void ESPMQTTHelper::loop() {
  mqttClient->handle();
}
