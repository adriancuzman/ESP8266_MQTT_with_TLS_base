#include <ESPMQTTHelper.h>

/**
Example for ESPMQTTHelper

The base/common code for sending and receiving information over MQTT on an ESP8266 with TLS encryption.
This base can be used to quickly add a sensor or control a device over MQTT.
Just add this library to have wifi with mDNS support and MQTT with TLS support.
This is used to have smaller .ino files and not duplicate the wifi+mqtt setup code, just add sensor/device specific code.

Configuration details will be loaded from file /config.json

Example of a configuration file:
{
  "wifi_ssid":"network_name",
  "wifi_password":"wifi_password",
  "mqtt_server":"hostname/url/ip/rasberrypi.local",
  "mqtt_server_port": 1883,
  "mqtt_user_name":"mqtt_user",
  "mqtt_user_password":"mqtt_user_password",
  "mqtt_client_name":"a client name, also used for mDNS ",
  "mqtt_input_topic":"default_input_topic_name",
  "mqtt_output_topic":"default_output_topic_name",
  "mqtt_server_fingerprint":"SHA1 fingerprint of the TLS certificate"
}
*/

// clients used that need to be instatiated outside
WiFiClientSecure espClient;
WiFiUDP udp;
PubSubClient pubSubClient(espClient);

/** mDNS resolver is used to resolve local hosts,
for example if your raspberrypi localhost is "rasberrypi.local"
The library can be found on github - https://github.com/madpilot/mDNSResolver
*/
mDNSResolver::Resolver resolver(udp);

ESPMQTTHelper mqttClient(&espClient, pubSubClient, &resolver);

void setup() {
  Serial.begin(115200);
  mqttClient.setup(onMessageReceived);
}

void onMessageReceived(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

}

long lastMsg = 0;
char msg[50];
int value = 0;

void loop() {
  mqttClient.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    mqttClient.sendMessage(msg);
  }
}
