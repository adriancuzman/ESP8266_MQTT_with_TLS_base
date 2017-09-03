#include "ESP8266MQTTClient.h"

using namespace mDNSResolver;

void ESP8266MQTTClient::setup(MQTT_CALLBACK_SIGNATURE) {
  setup_wifi();
  setup_mDNS();
  setup_mqtt_details(callback);
}

void ESP8266MQTTClient::setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  if (loadConfigFile()) {
    Serial.println("Config file loaded.");
  }
  else {
    Serial.println("Error loading config file.");
  }
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid.c_str());

  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void ESP8266MQTTClient::setup_mDNS() {
  if (!MDNS.begin(mqtt_client_name.c_str())) {
    // Start the mDNS responder for esp8266.local
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");
}


 
void ESP8266MQTTClient::resolve_mqtt_server_hostname() {
  if (mqtt_server.endsWith(".local")) {
    Serial.println("resolving using mDNS resolver because local hostname");
    
    mDNSResolver->setLocalIP(WiFi.localIP());
    mqtt_server_ip = mDNSResolver->search(mqtt_server.c_str());
    Serial.println(mqtt_server_ip);
    
  }
  else {
    Serial.println("resolving using WiFi.hostByName resolver because not local hostname");
    if (!WiFi.hostByName(mqtt_server.c_str(), mqtt_server_ip)) {
      Serial.println("Can't resolve mqtt_server hostname");
      while (1);
    }
  }

  if (mqtt_server_ip != INADDR_NONE) {
    Serial.print("MQTT Server Resolved: ");
    Serial.println(mqtt_server_ip);
  } else {
    Serial.println("Can't resolve mqtt_server hostname. Halt!");
    while (1);
  }
}

void ESP8266MQTTClient::setup_mqtt_details(MQTT_CALLBACK_SIGNATURE) {
  resolve_mqtt_server_hostname();
  pubSubClient.setServer(mqtt_server_ip, mqtt_server_port);
  pubSubClient.setCallback(callback);
}

void ESP8266MQTTClient::sendMessage(const char* payload) {
  pubSubClient.publish(mqtt_output_topic.c_str(), payload);
}

void ESP8266MQTTClient::sendMessage(const char* topic, const char* payload) {
  pubSubClient.publish(topic, payload);
}

void ESP8266MQTTClient::reconnect() {

  // Loop until we're reconnected
  int retryCounter = 0;
  while (!pubSubClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (pubSubClient.connect(mqtt_client_name.c_str(), mqtt_user_name.c_str(), mqtt_user_password.c_str())) {
      retryCounter = 0;
      Serial.println("connected");
      pubSubClient.subscribe(mqtt_input_topic.c_str());
    } else {
      retryCounter++;
      if (retryCounter == 3){
        ESP.restart();
      }
      Serial.print("failed, rc=");
      Serial.print(pubSubClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }

  if (secureClient->verify(mqtt_server_fingerprint.c_str(), mqtt_server.c_str())) {
    Serial.println("certificate matches");
  } else {
    Serial.print("Expected csr: #");Serial.print(mqtt_server_fingerprint.c_str());Serial.println("#");
    Serial.print("Expected host:");Serial.println(mqtt_server.c_str());
    
    
    Serial.println("certificate doesn't match... crashing...");
    while (1);
  }
}

bool ESP8266MQTTClient::loadConfigFile() {
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return false;
  }
  else {
    Serial.println("File system mounted.");
  }

  File configFile = SPIFFS.open(config_file, "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println("Failed to parse config file");
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

void ESP8266MQTTClient::readField(JsonObject *json, char* field_name, String &storeVariable) {
  if (json->containsKey(field_name)) {
    const char* field_value = (*json)[field_name];
    storeVariable = String(field_value);
  }
}

void ESP8266MQTTClient::readField(JsonObject *json, char* field_name, int &storeVariable) {
  if (json->containsKey(field_name)) {
    storeVariable = (*json)[field_name];
  }
}

void ESP8266MQTTClient::loop() {
  if (!pubSubClient.connected()) {
    reconnect();
  }
  pubSubClient.loop();
}
